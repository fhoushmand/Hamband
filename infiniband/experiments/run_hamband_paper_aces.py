#!/usr/bin/env python3
"""Run and validate the SafarDB Hamband matrix inside one ACES allocation."""

from __future__ import annotations

import argparse
import csv
import hashlib
import os
import re
import shlex
import socket
import statistics
import subprocess
import sys
import time
from concurrent.futures import ThreadPoolExecutor, as_completed
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable, Sequence

INFINIBAND_ROOT = Path(__file__).resolve().parents[1]
REPO_ROOT = INFINIBAND_ROOT.parent
DEFAULT_OPERATIONS = 4_000_000
DEFAULT_REPLICAS = (3, 4, 5, 6, 7, 8)
BASE_PERCENTAGES = (0, 15, 20, 25)
EXTRA_PERCENTAGES = (5, 50)
MAX_REPLICAS = max(DEFAULT_REPLICAS)


@dataclass(frozen=True)
class Workload:
    figure: int
    paper_name: str
    usecase: str
    kind: str
    generator: str

    def percentages(self) -> tuple[int, ...]:
        if self.usecase in {"kvstore", "smallbank"}:
            return tuple(sorted((*BASE_PERCENTAGES, *EXTRA_PERCENTAGES)))
        return BASE_PERCENTAGES


WORKLOADS = (
    Workload(9, "Counter", "counter", "CRDT", "counter-benchmark"),
    Workload(9, "Register", "register", "CRDT", "register-crdt-benchmark"),
    Workload(9, "G-Set", "gset", "CRDT", "gset-benchmark"),
    Workload(9, "PN-Set", "pnset", "CRDT", "pnset-benchmark"),
    Workload(9, "2P-Set", "twopset", "CRDT", "twopset-benchmark"),
    Workload(10, "Account", "account", "WRDT", "account-benchmark"),
    Workload(10, "Courseware", "courseware", "WRDT", "courseware-benchmark"),
    Workload(10, "Project", "project", "WRDT", "project-benchmark"),
    Workload(10, "Movie", "movie", "WRDT", "movie-benchmark"),
    Workload(10, "Auction", "rubis", "WRDT", "rubis-benchmark"),
    Workload(11, "YCSB", "kvstore", "CRDT", "kvstore-benchmark"),
    Workload(11, "SmallBank", "smallbank", "WRDT", "smallbank-benchmark"),
)

NODE_RESPONSE_FIELDS = tuple(
    f"response_node{node}_us" for node in range(1, MAX_REPLICAS + 1)
)
NODE_THROUGHPUT_FIELDS = tuple(
    f"throughput_node{node}_ops_per_us" for node in range(1, MAX_REPLICAS + 1)
)
CSV_FIELDS = (
    "paper_figure",
    "paper_workload",
    "repo_usecase",
    "rdt_kind",
    "transport",
    "commit",
    "slurm_job_id",
    "fixed_node_set",
    "registry_node",
    "replica_nodes",
    "replicas",
    "operations",
    "write_percentage",
    "response_time_avg_us",
    "throughput_min_ops_per_us",
    "throughput_min_ops_per_s",
    *NODE_RESPONSE_FIELDS,
    *NODE_THROUGHPUT_FIELDS,
    "issued_operations_total",
    "workload_digest",
    "state_digest",
    "measurement_runs",
    "status",
)

BAD_LOG_PATTERNS = (
    re.compile(r"segmentation fault", re.IGNORECASE),
    re.compile(r"core dumped", re.IGNORECASE),
    re.compile(r"terminate called", re.IGNORECASE),
    re.compile(r"what\(\):", re.IGNORECASE),
    re.compile(r"\baborted\b", re.IGNORECASE),
    re.compile(r"WRDT request failed", re.IGNORECASE),
    re.compile(r"not permissible, dropping", re.IGNORECASE),
    re.compile(r"integrity drops:\s*[1-9]", re.IGNORECASE),
    re.compile(r"dory errors:\s*[1-9]", re.IGNORECASE),
    re.compile(r"malformed method-call", re.IGNORECASE),
    re.compile(r"failed to (post|poll|open|query|create|connect)", re.IGNORECASE),
)


def git_commit() -> str:
    result = subprocess.run(
        ["git", "-C", str(REPO_ROOT), "rev-parse", "HEAD"],
        capture_output=True,
        text=True,
        check=False,
    )
    if result.returncode != 0:
        raise RuntimeError("Cannot determine the deployed Hamband commit")
    return result.stdout.strip()


def command_output(command: Sequence[str], timeout: int = 30) -> str:
    result = subprocess.run(
        list(command), capture_output=True, text=True, timeout=timeout, check=False
    )
    if result.returncode != 0:
        raise RuntimeError(
            f"Command failed ({shlex.join(command)}):\n{result.stdout}\n{result.stderr}"
        )
    return result.stdout.strip()


def allocation_nodes() -> tuple[str, ...]:
    node_list = os.environ.get("SLURM_JOB_NODELIST")
    if not node_list:
        raise RuntimeError("This runner must execute inside a Slurm allocation")
    nodes = tuple(
        line.strip()
        for line in command_output(["scontrol", "show", "hostnames", node_list]).splitlines()
        if line.strip()
    )
    if len(nodes) != MAX_REPLICAS + 1:
        raise RuntimeError(
            f"Expected one registry plus {MAX_REPLICAS} workers, found {len(nodes)} nodes"
        )
    if len(set(nodes)) != len(nodes):
        raise RuntimeError("Slurm allocation contains duplicate node names")
    return nodes


def step_command(node: str, script: str, cpus: int = 1) -> list[str]:
    return [
        "srun",
        "--overlap",
        "--exact",
        "--nodes=1",
        "--ntasks=1",
        f"--nodelist={node}",
        f"--cpus-per-task={cpus}",
        "--cpu-bind=cores",
        "bash",
        "-lc",
        script,
    ]


def run_step(node: str, script: str, timeout: int = 60, cpus: int = 1) -> str:
    return command_output(step_command(node, script, cpus), timeout)


def parallel_steps(nodes: Iterable[str], script: str, timeout: int = 60) -> list[str]:
    node_list = list(nodes)
    outputs: list[str | None] = [None] * len(node_list)
    with ThreadPoolExecutor(max_workers=len(node_list)) as executor:
        futures = {
            executor.submit(run_step, node, script, timeout): index
            for index, node in enumerate(node_list)
        }
        for future in as_completed(futures):
            outputs[futures[future]] = future.result()
    return [output for output in outputs if output is not None]


def verify_allocation(nodes: Sequence[str], commit: str) -> None:
    source_diff = subprocess.run(
        [
            "git",
            "-C",
            str(REPO_ROOT),
            "diff",
            "--quiet",
            "HEAD",
            "--",
            ".",
            ":(exclude)infiniband/results/**",
        ],
        check=False,
    )
    if source_diff.returncode != 0:
        raise RuntimeError(
            "Tracked source differs from the deployed commit; refusing an untraceable run"
        )
    if git_commit() != commit:
        raise RuntimeError("Hamband commit changed during allocation verification")
    for binary in ("band", "band-crdt"):
        path = INFINIBAND_ROOT / "wellcoordination" / "build" / "bin" / binary
        if not path.is_file() or not os.access(path, os.X_OK):
            raise RuntimeError(f"Missing executable: {path}")

    script = r'''
set -euo pipefail
test "$(numactl -H 2>/dev/null | awk '/available:/ {print $2; exit}')" = "2"
ibv_devinfo 2>/dev/null | grep -q 'state:[[:space:]]*PORT_ACTIVE'
ibv_devinfo 2>/dev/null | grep -q 'link_layer:[[:space:]]*InfiniBand'
printf '%s numa=2 infiniband=active\n' "$(hostname -s)"
'''
    outputs = parallel_steps(nodes, script)
    if len(outputs) != len(nodes):
        raise RuntimeError("Not every allocated node passed hardware verification")
    for output in outputs:
        print(f"[allocation] {output}", flush=True)


def registry_address(node: str) -> str:
    output = run_step(
        node,
        "hostname -I | awk '{for (i=1; i<=NF; i++) if ($i ~ /^[0-9]+\\./) {print $i; exit}}'",
    )
    if not re.fullmatch(r"\d+\.\d+\.\d+\.\d+", output):
        try:
            output = socket.gethostbyname(node)
        except socket.gaierror as error:
            raise RuntimeError(f"Cannot resolve registry node {node}") from error
    return output


class Registry:
    def __init__(self, node: str, address: str, port: int, memcached_bin: str):
        self.node = node
        self.address = address
        self.port = port
        self.memcached_bin = memcached_bin
        self.process: subprocess.Popen[bytes] | None = None
        self.log_handle = None

    @property
    def endpoint(self) -> str:
        return f"{self.address}:{self.port}"

    def start(self, log_path: Path) -> None:
        self.stop()
        log_path.parent.mkdir(parents=True, exist_ok=True)
        self.log_handle = log_path.open("wb")
        script = f'''
set -euo pipefail
memcached_bin={shlex.quote(self.memcached_bin)}
if [[ ! -x "$memcached_bin" ]]; then
  memcached_bin=$(command -v memcached)
fi
exec "$memcached_bin" -p {self.port} -U 0 -m 64
'''
        self.process = subprocess.Popen(
            step_command(self.node, script),
            stdout=self.log_handle,
            stderr=subprocess.STDOUT,
        )
        deadline = time.monotonic() + 30
        while time.monotonic() < deadline:
            if self.process.poll() is not None:
                self.stop()
                raise RuntimeError(f"memcached exited during startup; see {log_path}")
            try:
                with socket.create_connection((self.address, self.port), timeout=1):
                    return
            except OSError:
                time.sleep(0.25)
        self.stop()
        raise RuntimeError("Timed out waiting for the experiment registry")

    def stop(self) -> None:
        if self.process is not None and self.process.poll() is None:
            self.process.terminate()
            try:
                self.process.wait(timeout=10)
            except subprocess.TimeoutExpired:
                self.process.kill()
                self.process.wait(timeout=10)
        self.process = None
        if self.log_handle is not None:
            self.log_handle.close()
            self.log_handle = None


def generate_workload(
    workload: Workload, replicas: int, operations: int, percentage: int
) -> tuple[Path, str]:
    workload_root = INFINIBAND_ROOT / "wellcoordination" / "workload"
    directory = workload_root / f"{replicas}-{operations}-{percentage}" / workload.usecase
    if directory.exists():
        for path in directory.iterdir():
            if path.is_file():
                path.unlink()
        directory.rmdir()
    env = os.environ.copy()
    env["HAMBAND_WORKLOAD_DIR"] = str(workload_root)
    generator = (
        INFINIBAND_ROOT
        / "wellcoordination"
        / "build"
        / "bin"
        / workload.generator
    )
    result = subprocess.run(
        [str(generator), str(replicas), str(operations), str(percentage)],
        env=env,
        capture_output=True,
        text=True,
        timeout=1800,
        check=False,
    )
    if result.returncode != 0:
        raise RuntimeError(
            f"Workload generation failed:\n{result.stdout}\n{result.stderr}"
        )

    files = [directory / f"{node}.txt" for node in range(1, replicas + 1)]
    if any(not path.is_file() for path in files):
        raise RuntimeError(f"Generator did not create exactly {replicas} workload files")
    expected_writes = operations * percentage // 100
    calls = 0
    digest = hashlib.sha256()
    for path in files:
        lines = path.read_text(encoding="utf-8").splitlines()
        if not lines or lines[0] != f"#{expected_writes}":
            raise RuntimeError(f"Invalid expected-write header in {path}")
        calls += sum(1 for line in lines[1:] if line)
        digest.update(path.name.encode("ascii"))
        digest.update(b"\0")
        digest.update(path.read_bytes())
    if calls != operations:
        raise RuntimeError(f"Generator emitted {calls} calls instead of {operations}")
    return directory, digest.hexdigest()


def remove_workload(directory: Path) -> None:
    if not directory.exists():
        return
    for path in directory.iterdir():
        if path.is_file():
            path.unlink()
    directory.rmdir()


def stop_processes(processes: Iterable[subprocess.Popen[bytes]]) -> None:
    running = [process for process in processes if process.poll() is None]
    for process in running:
        process.terminate()
    deadline = time.monotonic() + 10
    while running and time.monotonic() < deadline:
        running = [process for process in running if process.poll() is None]
        time.sleep(0.1)
    for process in running:
        process.kill()
    for process in running:
        try:
            process.wait(timeout=5)
        except subprocess.TimeoutExpired:
            pass


def validate_logs(outputs: Sequence[str], replicas: int, run_name: str, operations: int) -> None:
    if len(outputs) != replicas:
        raise RuntimeError(f"{run_name}: expected {replicas} logs, found {len(outputs)}")
    issued: list[int] = []
    digests: list[str] = []
    for node_id, output in enumerate(outputs, start=1):
        if output.count("all nodes finished") != 1:
            raise RuntimeError(f"{run_name}: node {node_id} missed the finish barrier")
        for pattern in BAD_LOG_PATTERNS:
            if pattern.search(output):
                raise RuntimeError(
                    f"{run_name}: node {node_id} matched error pattern {pattern.pattern}"
                )
        issued_matches = re.findall(r"^issued\s+(\d+)\s+operations$", output, re.MULTILINE)
        digest_matches = re.findall(r"^state digest:\s*(\d+)\s*$", output, re.MULTILINE)
        if len(issued_matches) != 1 or len(digest_matches) != 1:
            raise RuntimeError(f"{run_name}: node {node_id} has incomplete final metrics")
        issued.append(int(issued_matches[0]))
        digests.append(digest_matches[0])
    if sum(issued) != operations:
        raise RuntimeError(f"{run_name}: replicas issued {sum(issued)} operations")
    if len(set(digests)) != 1:
        raise RuntimeError(f"{run_name}: replicas did not converge: {digests}")


def extract_values(outputs: Sequence[str], pattern: str, label: str) -> list[float]:
    compiled = re.compile(pattern, re.MULTILINE)
    values: list[float] = []
    for node_id, output in enumerate(outputs, start=1):
        matches = compiled.findall(output)
        if len(matches) != 1:
            raise RuntimeError(
                f"Expected one {label} value on node {node_id}, found {len(matches)}"
            )
        values.append(float(matches[0]))
    return values


def run_replicas(
    workload: Workload,
    replicas: int,
    operations: int,
    percentage: int,
    mode: str,
    worker_nodes: Sequence[str],
    registry: Registry,
    log_root: Path,
    timeout: int,
) -> list[str]:
    run_name = f"fig{workload.figure}-{workload.usecase}-r{replicas}-w{percentage}-{mode}"
    run_dir = log_root / run_name
    run_dir.mkdir(parents=True, exist_ok=True)
    registry.start(run_dir / "registry.log")

    processes: list[subprocess.Popen[bytes]] = []
    log_handles = []
    try:
        for node_id, node in enumerate(worker_nodes[:replicas], start=1):
            if workload.kind == "CRDT":
                binary = "band-crdt"
                arguments = [node_id, replicas, operations, percentage, workload.usecase, 0]
            else:
                binary = "band"
                throughput_flag = 1 if mode == "throughput" else 0
                arguments = [
                    node_id,
                    replicas,
                    operations,
                    percentage,
                    workload.usecase,
                    throughput_flag,
                    0,
                ]
            invocation = shlex.join(
                [
                    str(INFINIBAND_ROOT / "wellcoordination" / "build" / "bin" / binary),
                    *(str(value) for value in arguments),
                ]
            )
            script = f'''
set -euo pipefail
export DORY_REGISTRY_IP={shlex.quote(registry.endpoint)}
export HAMBAND_WORKLOAD_DIR={shlex.quote(str(INFINIBAND_ROOT / "wellcoordination" / "workload"))}
exec timeout --signal=TERM --kill-after=10s {timeout}s {invocation}
'''
            handle = (run_dir / f"node{node_id}.log").open("wb")
            log_handles.append(handle)
            processes.append(
                subprocess.Popen(
                    step_command(node, script, cpus=4),
                    stdout=handle,
                    stderr=subprocess.STDOUT,
                )
            )

        deadline = time.monotonic() + timeout + 120
        while any(process.poll() is None for process in processes):
            if time.monotonic() >= deadline:
                stop_processes(processes)
                raise RuntimeError(f"{run_name}: Slurm steps exceeded the timeout")
            if any(
                process.poll() not in (None, 0)
                for process in processes
            ):
                stop_processes(processes)
                break
            time.sleep(0.5)

        return_codes = [process.wait() for process in processes]
        if any(code != 0 for code in return_codes):
            raise RuntimeError(f"{run_name}: worker exit codes were {return_codes}")
    finally:
        stop_processes(processes)
        for handle in log_handles:
            handle.close()
        registry.stop()

    outputs = [
        (run_dir / f"node{node_id}.log").read_text(encoding="utf-8", errors="replace")
        for node_id in range(1, replicas + 1)
    ]
    validate_logs(outputs, replicas, run_name, operations)
    return outputs


def state_digest(outputs: Sequence[str]) -> str:
    matches = re.findall(r"^state digest:\s*(\d+)\s*$", outputs[0], re.MULTILINE)
    if len(matches) != 1:
        raise RuntimeError("Cannot extract the validated state digest")
    return matches[0]


def make_row(
    workload: Workload,
    replicas: int,
    operations: int,
    percentage: int,
    response_outputs: Sequence[str],
    throughput_outputs: Sequence[str],
    workload_digest: str,
    commit: str,
    job_id: str,
    nodes: Sequence[str],
) -> dict[str, object]:
    response_values = extract_values(
        response_outputs,
        r"^total average response time for \d+ calls:\s*([0-9.eE+-]+)\s*$",
        "response time",
    )
    throughput_values = extract_values(
        throughput_outputs,
        r"^throughput:\s*([0-9.eE+-]+)\s*$",
        "throughput",
    )
    response_digest = state_digest(response_outputs)
    throughput_digest = state_digest(throughput_outputs)
    if response_digest != throughput_digest:
        raise RuntimeError("Response and throughput runs reached different final states")
    issued_total = sum(
        int(value)
        for output in response_outputs
        for value in re.findall(r"^issued\s+(\d+)\s+operations$", output, re.MULTILINE)
    )
    minimum_throughput = min(throughput_values)
    row: dict[str, object] = {
        "paper_figure": workload.figure,
        "paper_workload": workload.paper_name,
        "repo_usecase": workload.usecase,
        "rdt_kind": workload.kind,
        "transport": "InfiniBand",
        "commit": commit,
        "slurm_job_id": job_id,
        "fixed_node_set": ";".join(nodes),
        "registry_node": nodes[0],
        "replica_nodes": ";".join(nodes[1 : replicas + 1]),
        "replicas": replicas,
        "operations": operations,
        "write_percentage": percentage,
        "response_time_avg_us": f"{statistics.fmean(response_values):.9f}",
        "throughput_min_ops_per_us": f"{minimum_throughput:.9f}",
        "throughput_min_ops_per_s": f"{minimum_throughput * 1_000_000:.3f}",
        "issued_operations_total": issued_total,
        "workload_digest": workload_digest,
        "state_digest": response_digest,
        "measurement_runs": "combined" if workload.kind == "CRDT" else "split",
        "status": "valid",
    }
    for node in range(1, MAX_REPLICAS + 1):
        row[f"response_node{node}_us"] = (
            f"{response_values[node - 1]:.9f}" if node <= replicas else ""
        )
        row[f"throughput_node{node}_ops_per_us"] = (
            f"{throughput_values[node - 1]:.9f}" if node <= replicas else ""
        )
    return row


def load_rows(path: Path) -> list[dict[str, str]]:
    if not path.exists():
        return []
    with path.open(newline="", encoding="utf-8") as source:
        return list(csv.DictReader(source))


def save_rows(path: Path, rows: Sequence[dict[str, object]]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    with temporary.open("w", newline="", encoding="utf-8") as destination:
        writer = csv.DictWriter(destination, fieldnames=CSV_FIELDS)
        writer.writeheader()
        writer.writerows(rows)
    temporary.replace(path)


def row_key(row: dict[str, object]) -> tuple[str, int, int]:
    return (
        str(row["repo_usecase"]),
        int(row["replicas"]),
        int(row["write_percentage"]),
    )


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--workload",
        nargs="+",
        choices=[workload.usecase for workload in WORKLOADS],
    )
    parser.add_argument("--replicas", nargs="+", type=int, choices=DEFAULT_REPLICAS)
    parser.add_argument("--percentage", nargs="+", type=int, choices=range(101))
    parser.add_argument("--operations", type=int, default=DEFAULT_OPERATIONS)
    parser.add_argument("--timeout", type=int, default=3600)
    parser.add_argument("--attempts", type=int, default=2)
    parser.add_argument("--reuse-commit", action="append", default=[])
    parser.add_argument("--output", type=Path)
    parser.add_argument("--log-dir", type=Path)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.operations <= 0 or args.attempts <= 0:
        raise RuntimeError("Operations and attempts must be positive")

    commit = git_commit()
    job_id = os.environ.get("SLURM_JOB_ID", "unknown")
    nodes = allocation_nodes()
    registry_node = nodes[0]
    worker_nodes = nodes[1:]
    node_set = ";".join(nodes)
    verify_allocation(nodes, commit)

    default_name = (
        "hamband_infiniband_aces_4m.csv"
        if args.operations == DEFAULT_OPERATIONS
        else f"hamband_infiniband_aces_{args.operations}.csv"
    )
    csv_path = args.output or INFINIBAND_ROOT / "results" / default_name
    log_root = args.log_dir or csv_path.parent / f"{csv_path.stem}_logs"
    csv_path = csv_path.resolve()
    log_root = log_root.resolve()
    log_root.mkdir(parents=True, exist_ok=True)

    selected_names = set(args.workload or [workload.usecase for workload in WORKLOADS])
    selected_workloads = [
        workload for workload in WORKLOADS if workload.usecase in selected_names
    ]
    selected_replicas = tuple(args.replicas or DEFAULT_REPLICAS)
    requested_percentages = set(args.percentage) if args.percentage else None

    matrix: list[tuple[Workload, int, int]] = []
    for workload in selected_workloads:
        percentages = workload.percentages()
        if requested_percentages is not None:
            percentages = tuple(
                percentage
                for percentage in percentages
                if percentage in requested_percentages
            )
            unsupported = requested_percentages - set(workload.percentages())
            if unsupported:
                raise RuntimeError(
                    f"Unsupported percentages for {workload.usecase}: {sorted(unsupported)}"
                )
        for replicas in selected_replicas:
            for percentage in percentages:
                matrix.append((workload, replicas, percentage))

    accepted_commits = {commit, *args.reuse_commit}
    rows: list[dict[str, object]] = [
        dict(row)
        for row in load_rows(csv_path)
        if row.get("status") == "valid"
        and row.get("commit") in accepted_commits
        and row.get("fixed_node_set") == node_set
        and int(row.get("operations", 0)) == args.operations
    ]
    completed = {row_key(row) for row in rows}
    pending = [item for item in matrix if row_key({
        "repo_usecase": item[0].usecase,
        "replicas": item[1],
        "write_percentage": item[2],
    }) not in completed]

    expected_full_rows = 312
    print(
        f"[matrix] commit={commit} job={job_id} registry={registry_node} "
        f"workers={','.join(worker_nodes)}",
        flush=True,
    )
    print(
        f"[matrix] {len(pending)} pending of {len(matrix)} selected configurations; "
        f"full matrix size={expected_full_rows}",
        flush=True,
    )

    port = int(os.environ.get("HAMBAND_REGISTRY_PORT", 19000 + int(job_id) % 10000))
    registry = Registry(
        registry_node,
        registry_address(registry_node),
        port,
        os.environ.get(
            "HAMBAND_MEMCACHED_BIN",
            "/scratch/user/u.js213354/memcached/bin/memcached",
        ),
    )

    try:
        for index, (workload, replicas, percentage) in enumerate(pending, start=1):
            label = (
                f"[{index}/{len(pending)}] fig{workload.figure} {workload.paper_name} "
                f"r{replicas} w{percentage}"
            )
            print(f"{label}: generating exact workload", flush=True)
            directory, workload_hash = generate_workload(
                workload, replicas, args.operations, percentage
            )
            last_error: Exception | None = None
            try:
                for attempt in range(1, args.attempts + 1):
                    attempt_logs = log_root / f"attempt-{attempt}"
                    try:
                        if workload.kind == "CRDT":
                            outputs = run_replicas(
                                workload,
                                replicas,
                                args.operations,
                                percentage,
                                "combined",
                                worker_nodes,
                                registry,
                                attempt_logs,
                                args.timeout,
                            )
                            response_outputs = outputs
                            throughput_outputs = outputs
                        else:
                            response_outputs = run_replicas(
                                workload,
                                replicas,
                                args.operations,
                                percentage,
                                "response",
                                worker_nodes,
                                registry,
                                attempt_logs,
                                args.timeout,
                            )
                            throughput_outputs = run_replicas(
                                workload,
                                replicas,
                                args.operations,
                                percentage,
                                "throughput",
                                worker_nodes,
                                registry,
                                attempt_logs,
                                args.timeout,
                            )
                        row = make_row(
                            workload,
                            replicas,
                            args.operations,
                            percentage,
                            response_outputs,
                            throughput_outputs,
                            workload_hash,
                            commit,
                            job_id,
                            nodes,
                        )
                        rows.append(row)
                        rows.sort(
                            key=lambda item: (
                                int(item["paper_figure"]),
                                str(item["repo_usecase"]),
                                int(item["replicas"]),
                                int(item["write_percentage"]),
                            )
                        )
                        save_rows(csv_path, rows)
                        print(
                            f"{label}: valid response={row['response_time_avg_us']} us "
                            f"throughput={row['throughput_min_ops_per_s']} ops/s",
                            flush=True,
                        )
                        last_error = None
                        break
                    except Exception as error:
                        last_error = error
                        registry.stop()
                        print(
                            f"{label}: attempt {attempt}/{args.attempts} failed: {error}",
                            file=sys.stderr,
                            flush=True,
                        )
                        if attempt < args.attempts:
                            time.sleep(5)
                if last_error is not None:
                    raise last_error
            finally:
                remove_workload(directory)
    finally:
        registry.stop()

    print(f"[matrix] completed {len(rows)} valid rows in {csv_path}", flush=True)
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as error:
        print(f"ERROR: {error}", file=sys.stderr, flush=True)
        raise

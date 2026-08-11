#!/usr/bin/env python3
"""Run and validate the four-replica Hamband failure matrix on ACES."""

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
import time
from concurrent.futures import ThreadPoolExecutor, as_completed
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable, Sequence


ROOT = Path(__file__).resolve().parents[1]
REPO_ROOT = ROOT.parent
BINARY_DIR = ROOT / "wellcoordination" / "build" / "bin"
WORKLOAD_ROOT = ROOT / "wellcoordination" / "workload"
BASELINE_CSV = ROOT / "results" / "hamband_infiniband_aces_4m.csv"
OPERATIONS = 4_000_000
REPLICAS = 4
PERCENTAGES = (15, 20, 25)
PROCESS_CPUS = 4
RUNTIME_PATH = (
    "/scratch/user/u.js213354/libmemcached/build/lib:"
    "/sw/eb/sw/GCCcore/8.3.0/lib64"
)
LIBSTDCPP_SHA256 = "cd83a7033636810b5b9be5f3b4702d79a8adb9fd7d42ddf64e9988635a86798a"
LIBGCC_SHA256 = "44d951ea7184dfc82128451e23f8b15a9d9aae4376e1a39ce2373f6e2cb40299"

NODE_RESPONSE_FIELDS = tuple(f"response_node{node}_us" for node in range(1, 9))
NODE_THROUGHPUT_FIELDS = tuple(
    f"throughput_node{node}_ops_per_us" for node in range(1, 9)
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
    "failure_scenario",
    "failed_node",
    "failure_injection",
    "surviving_replicas",
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

BAD_PATTERNS = (
    re.compile(r"segmentation fault", re.IGNORECASE),
    re.compile(r"core dumped", re.IGNORECASE),
    re.compile(r"terminate called", re.IGNORECASE),
    re.compile(r"what\(\):", re.IGNORECASE),
    re.compile(r"\baborted\b", re.IGNORECASE),
    re.compile(r"not permissible, dropping", re.IGNORECASE),
    re.compile(r"WRDT request failed", re.IGNORECASE),
    re.compile(r"failed to (post|poll|open|query|create|connect)", re.IGNORECASE),
)


@dataclass(frozen=True)
class Scenario:
    paper_name: str
    usecase: str
    kind: str
    generator: str
    binary: str
    failure_scenario: str
    failed_node: int


SCENARIOS = (
    Scenario(
        "Account follower-failure",
        "account",
        "WRDT",
        "account-benchmark",
        "band-failure",
        "follower-failure",
        2,
    ),
    Scenario(
        "Account leader-failure",
        "account",
        "WRDT",
        "account-benchmark",
        "band-failure",
        "leader-failure",
        1,
    ),
    Scenario(
        "2P-Set replica-failure",
        "twopset",
        "CRDT",
        "twopset-benchmark",
        "band-crdt-failure",
        "replica-failure",
        1,
    ),
)


def command_output(command: Sequence[str], timeout: int = 30) -> str:
    result = subprocess.run(
        list(command), capture_output=True, text=True, timeout=timeout, check=False
    )
    if result.returncode != 0:
        raise RuntimeError(
            f"Command failed ({shlex.join(command)}):\n{result.stdout}\n{result.stderr}"
        )
    return result.stdout.strip()


def file_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def git_commit() -> str:
    return command_output(["git", "-C", str(REPO_ROOT), "rev-parse", "HEAD"])


def allocation_nodes() -> tuple[str, ...]:
    node_list = os.environ.get("SLURM_JOB_NODELIST")
    if not node_list:
        raise RuntimeError("The failure runner must execute inside Slurm")
    nodes = tuple(
        line.strip()
        for line in command_output(
            ["scontrol", "show", "hostnames", node_list]
        ).splitlines()
        if line.strip()
    )
    if len(nodes) != REPLICAS + 1 or len(set(nodes)) != len(nodes):
        raise RuntimeError(
            f"Expected one registry and four distinct workers, found {nodes}"
        )
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


def verify_runtime() -> None:
    libraries: dict[str, Path] = {}
    for name in (
        "band-failure",
        "band-crdt-failure",
        "account-benchmark",
        "twopset-benchmark",
    ):
        binary = BINARY_DIR / name
        if not binary.is_file() or not os.access(binary, os.X_OK):
            raise RuntimeError(f"Missing executable: {binary}")
        env = os.environ.copy()
        env["LD_LIBRARY_PATH"] = RUNTIME_PATH
        result = subprocess.run(
            ["ldd", str(binary)],
            env=env,
            capture_output=True,
            text=True,
            timeout=30,
            check=False,
        )
        output = result.stdout + result.stderr
        if result.returncode != 0 or "not found" in output:
            raise RuntimeError(f"Unresolved runtime for {binary}:\n{output}")
        if name == "band-failure":
            for library in ("libstdc++.so.6", "libgcc_s.so.1"):
                match = re.search(
                    rf"^\s*{re.escape(library)}\s+=>\s+(\S+)",
                    output,
                    re.MULTILINE,
                )
                if not match:
                    raise RuntimeError(f"Cannot resolve {library} for {binary}")
                libraries[library] = Path(match.group(1)).resolve()
    actual = (
        file_sha256(libraries["libstdc++.so.6"]),
        file_sha256(libraries["libgcc_s.so.1"]),
    )
    expected = (LIBSTDCPP_SHA256, LIBGCC_SHA256)
    if actual != expected:
        raise RuntimeError(f"C++ runtime mismatch: actual={actual} expected={expected}")


def verify_allocation(worker_nodes: Sequence[str]) -> str:
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
        raise RuntimeError("Tracked source differs from the recorded commit")
    verify_runtime()
    probe = r'''
set -euo pipefail
test "$(numactl -H 2>/dev/null | awk '/available:/ {print $2; exit}')" = 2
test -d /sys/class/infiniband/mlx5_0
device_info=$(ibv_devinfo -d mlx5_0 2>/dev/null)
grep -q 'state:[[:space:]]*PORT_ACTIVE' <<<"$device_info"
grep -q 'link_layer:[[:space:]]*InfiniBand' <<<"$device_info"
model=$(lscpu | awk -F: '/^Model name:/ {sub(/^[[:space:]]+/, "", $2); print $2; exit}')
logical=$(lscpu -p=CPU | grep -vc '^#')
rate=$(ibstat mlx5_0 2>/dev/null | awk '/Rate:/ {print $2 $3; exit}')
nic_numa=$(cat /sys/class/infiniband/mlx5_0/device/numa_node)
printf '%s|model=%s|logical=%s|device=mlx5_0|nic_numa=%s|rate=%s\n' \
  "$(hostname -s)" "$model" "$logical" "$nic_numa" "$rate"
'''
    outputs = parallel_steps(worker_nodes, probe)
    signatures: list[str] = []
    for output in outputs:
        line = output.strip()
        fields = line.split("|", 1)
        if len(fields) != 2:
            raise RuntimeError(f"Malformed hardware signature: {line}")
        signatures.append(fields[1])
        print(f"[allocation] {line}", flush=True)
    if len(signatures) != len(worker_nodes) or len(set(signatures)) != 1:
        raise RuntimeError(f"Worker hardware differs: {outputs}")
    return signatures[0]


def registry_address(node: str) -> str:
    output = run_step(
        node,
        "hostname -I | awk '{for (i=1; i<=NF; i++) if ($i ~ /^[0-9]+\\./) {print $i; exit}}'",
        timeout=30,
    )
    if not re.fullmatch(r"\d+\.\d+\.\d+\.\d+", output):
        try:
            output = socket.gethostbyname(node)
        except socket.gaierror as error:
            raise RuntimeError(f"Cannot resolve registry node {node}") from error
    return output


class Registry:
    def __init__(self, node: str, address: str, port: int):
        self.node = node
        self.endpoint = f"{address}:{port}"
        self.port = port
        self.process: subprocess.Popen[bytes] | None = None
        self.log_handle = None

    def start(self, log_path: Path) -> None:
        self.stop()
        log_path.parent.mkdir(parents=True, exist_ok=True)
        self.log_handle = log_path.open("wb")
        memcached = os.environ.get(
            "HAMBAND_MEMCACHED_BIN",
            "/scratch/user/u.js213354/memcached/bin/memcached",
        )
        script = f'''
set -euo pipefail
binary={shlex.quote(memcached)}
if [[ ! -x "$binary" ]]; then binary=$(command -v memcached); fi
exec "$binary" -p {self.port} -U 0 -m 64
'''
        self.process = subprocess.Popen(
            step_command(self.node, script),
            stdout=self.log_handle,
            stderr=subprocess.STDOUT,
        )
        deadline = time.monotonic() + 30
        while time.monotonic() < deadline:
            if self.process.poll() is not None:
                raise RuntimeError(f"memcached exited; see {log_path}")
            try:
                address, port = self.endpoint.rsplit(":", 1)
                with socket.create_connection((address, int(port)), timeout=1):
                    return
            except OSError:
                time.sleep(0.25)
        raise RuntimeError("Timed out waiting for memcached")

    def stop(self) -> None:
        if self.process is not None and self.process.poll() is None:
            self.process.terminate()
            try:
                self.process.wait(timeout=10)
            except subprocess.TimeoutExpired:
                self.process.kill()
                self.process.wait(timeout=5)
        self.process = None
        if self.log_handle is not None:
            self.log_handle.close()
        self.log_handle = None


def generate_workload(scenario: Scenario, percentage: int) -> tuple[Path, str]:
    directory = WORKLOAD_ROOT / f"{REPLICAS}-{OPERATIONS}-{percentage}" / scenario.usecase
    if directory.exists():
        for path in directory.iterdir():
            if path.is_file():
                path.unlink()
        directory.rmdir()
    env = os.environ.copy()
    env["HAMBAND_WORKLOAD_DIR"] = str(WORKLOAD_ROOT)
    env["LD_LIBRARY_PATH"] = RUNTIME_PATH
    result = subprocess.run(
        [
            str(BINARY_DIR / scenario.generator),
            str(REPLICAS),
            str(OPERATIONS),
            str(percentage),
        ],
        env=env,
        capture_output=True,
        text=True,
        timeout=1800,
        check=False,
    )
    if result.returncode != 0:
        raise RuntimeError(f"Workload generation failed:\n{result.stdout}\n{result.stderr}")

    files = [directory / f"{node}.txt" for node in range(1, REPLICAS + 1)]
    digest = hashlib.sha256()
    calls = 0
    expected_writes = OPERATIONS * percentage // 100
    for path in files:
        lines = path.read_text(encoding="utf-8").splitlines()
        if not lines or lines[0] != f"#{expected_writes}":
            raise RuntimeError(f"Invalid workload header: {path}")
        calls += len([line for line in lines[1:] if line])
        digest.update(path.name.encode("ascii"))
        digest.update(b"\0")
        digest.update(path.read_bytes())
    if calls != OPERATIONS:
        raise RuntimeError(f"Generator emitted {calls} calls")
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


def run_replicas(
    scenario: Scenario,
    percentage: int,
    mode: str,
    worker_nodes: Sequence[str],
    registry: Registry,
    log_root: Path,
    timeout: int,
) -> list[str]:
    run_name = f"{scenario.usecase}-{scenario.failure_scenario}-w{percentage}-{mode}"
    run_dir = log_root / run_name
    run_dir.mkdir(parents=True, exist_ok=True)
    registry.start(run_dir / "registry.log")
    processes: list[subprocess.Popen[bytes]] = []
    handles = []
    try:
        for node_id, node in enumerate(worker_nodes, start=1):
            if scenario.kind == "WRDT":
                arguments = [
                    node_id,
                    REPLICAS,
                    OPERATIONS,
                    percentage,
                    scenario.usecase,
                    1 if mode == "throughput" else 0,
                    scenario.failed_node,
                ]
            else:
                arguments = [
                    node_id,
                    REPLICAS,
                    OPERATIONS,
                    percentage,
                    scenario.usecase,
                    scenario.failed_node,
                ]
            invocation = shlex.join(
                [str(BINARY_DIR / scenario.binary), *(str(value) for value in arguments)]
            )
            script = f'''
set -euo pipefail
export DORY_REGISTRY_IP={shlex.quote(registry.endpoint)}
export HAMBAND_WORKLOAD_DIR={shlex.quote(str(WORKLOAD_ROOT))}
export LD_LIBRARY_PATH={shlex.quote(RUNTIME_PATH)}
child=""
cleanup() {{
  if [[ -n "$child" ]]; then kill -KILL "$child" 2>/dev/null || true; fi
}}
trap cleanup TERM INT EXIT
{invocation} &
child=$!
wait "$child"
status=$?
trap - TERM INT EXIT
exit "$status"
'''
            handle = (run_dir / f"node{node_id}.log").open("wb")
            handles.append(handle)
            processes.append(
                subprocess.Popen(
                    step_command(node, script, PROCESS_CPUS),
                    stdout=handle,
                    stderr=subprocess.STDOUT,
                )
            )

        survivor_indexes = [
            index for index in range(REPLICAS) if index + 1 != scenario.failed_node
        ]
        deadline = time.monotonic() + timeout
        while any(processes[index].poll() is None for index in survivor_indexes):
            failed_survivors = [
                processes[index].poll()
                for index in survivor_indexes
                if processes[index].poll() not in (None, 0)
            ]
            if failed_survivors:
                raise RuntimeError(
                    f"{run_name}: survivor step failed with {failed_survivors}"
                )
            if time.monotonic() >= deadline:
                raise RuntimeError(f"{run_name}: survivor timeout")
            time.sleep(0.2)

        survivor_codes = [processes[index].wait() for index in survivor_indexes]
        if any(code != 0 for code in survivor_codes):
            raise RuntimeError(f"{run_name}: survivor exit codes {survivor_codes}")
        stop_processes([processes[scenario.failed_node - 1]])
    finally:
        stop_processes(processes)
        for handle in handles:
            handle.close()
        registry.stop()

    return [
        (run_dir / f"node{node}.log").read_text(encoding="utf-8", errors="replace")
        for node in range(1, REPLICAS + 1)
    ]


def one_match(output: str, pattern: str, label: str) -> str:
    matches = re.findall(pattern, output, re.MULTILINE)
    if len(matches) != 1:
        raise RuntimeError(f"Expected one {label}, found {len(matches)}")
    return matches[0]


def validate_outputs(
    outputs: Sequence[str], scenario: Scenario, mode: str
) -> tuple[list[int], str, int]:
    failed_index = scenario.failed_node - 1
    failed_output = outputs[failed_index]
    before_failure = int(
        one_match(
            failed_output,
            r"^issued before failure\s+(\d+)\s+operations$",
            "failed-node issued count",
        )
    )
    if failed_output.count("failure injected at node") != 1:
        raise RuntimeError("Failed replica did not record exactly one failure")
    failed_protocol_output = re.sub(
        r"^srun: Job step aborted: Waiting up to \d+ seconds for job step to finish\.\s*$",
        "",
        failed_output,
        flags=re.MULTILINE,
    )
    for pattern in BAD_PATTERNS:
        if pattern.search(failed_protocol_output):
            raise RuntimeError(f"Failed-node log matched {pattern.pattern}")

    survivor_ids = [node for node in range(1, REPLICAS + 1) if node != scenario.failed_node]
    survivor_totals: list[int] = []
    state_digests: list[str] = []
    redirected: list[int] = []
    for node in survivor_ids:
        output = outputs[node - 1]
        if output.count("all surviving nodes finished") != 1:
            raise RuntimeError(f"Node {node} missed the surviving finish barrier")
        if output.count("failure observed: node") != 1:
            raise RuntimeError(f"Node {node} did not observe the failure")
        for pattern in BAD_PATTERNS:
            if pattern.search(output):
                raise RuntimeError(f"Node {node} matched {pattern.pattern}")
        survivor_totals.append(
            int(
                one_match(
                    output,
                    r"^issued\s+(\d+)\s+total operations$",
                    f"node {node} issued count",
                )
            )
        )
        redirected.append(
            int(
                one_match(
                    output,
                    r"^redirected\s+(\d+)\s+operations$",
                    f"node {node} redirected count",
                )
            )
        )
        state_digests.append(
            one_match(output, r"^state digest:\s*(\d+)\s*$", f"node {node} digest")
        )
    issued_total = before_failure + sum(survivor_totals)
    if issued_total != OPERATIONS:
        raise RuntimeError(f"Issued {issued_total} operations instead of {OPERATIONS}")
    if sum(value > 0 for value in redirected) != 1:
        raise RuntimeError(f"Expected exactly one redirector: {redirected}")
    if len(set(state_digests)) != 1:
        raise RuntimeError(f"Survivors did not converge: {state_digests}")
    return survivor_ids, state_digests[0], issued_total


def extract_metrics(
    outputs: Sequence[str], survivor_ids: Sequence[int], metric: str
) -> list[float]:
    if metric == "response":
        pattern = r"^total average response time for \d+ calls:\s*([0-9.eE+-]+)\s*$"
    else:
        pattern = r"^throughput:\s*([0-9.eE+-]+)\s*$"
    return [
        float(one_match(outputs[node - 1], pattern, f"node {node} {metric}"))
        for node in survivor_ids
    ]


def load_baselines() -> dict[tuple[str, int], dict[str, str]]:
    with BASELINE_CSV.open(newline="", encoding="utf-8") as source:
        rows = list(csv.DictReader(source))
    baselines = {
        (row["repo_usecase"], int(row["write_percentage"])): row
        for row in rows
        if row["status"] == "valid"
        and int(row["replicas"]) == REPLICAS
        and row["repo_usecase"] in {"account", "twopset"}
        and int(row["write_percentage"]) in PERCENTAGES
    }
    if len(baselines) != 6:
        raise RuntimeError(f"Expected six ordinary baselines, found {len(baselines)}")
    return baselines


def make_row(
    scenario: Scenario,
    percentage: int,
    response_outputs: Sequence[str],
    throughput_outputs: Sequence[str],
    workload_digest: str,
    expected_state_digest: str,
    commit: str,
    job_id: str,
    nodes: Sequence[str],
    worker_nodes: Sequence[str],
) -> dict[str, object]:
    response_survivors, response_digest, issued_total = validate_outputs(
        response_outputs, scenario, "response"
    )
    throughput_survivors, throughput_digest, throughput_issued = validate_outputs(
        throughput_outputs, scenario, "throughput"
    )
    if response_survivors != throughput_survivors:
        raise RuntimeError("Response and throughput survivor sets differ")
    if issued_total != throughput_issued:
        raise RuntimeError("Response and throughput issued totals differ")
    if response_digest != throughput_digest or response_digest != expected_state_digest:
        raise RuntimeError(
            "Failure run state differs from response, throughput, or ordinary baseline: "
            f"response={response_digest} throughput={throughput_digest} "
            f"baseline={expected_state_digest}"
        )

    response_values = extract_metrics(response_outputs, response_survivors, "response")
    throughput_values = extract_metrics(
        throughput_outputs, throughput_survivors, "throughput"
    )
    minimum_throughput = min(throughput_values)
    row: dict[str, object] = {field: "" for field in CSV_FIELDS}
    row.update(
        {
            "paper_figure": 14,
            "paper_workload": scenario.paper_name,
            "repo_usecase": scenario.usecase,
            "rdt_kind": scenario.kind,
            "transport": "InfiniBand",
            "commit": commit,
            "slurm_job_id": job_id,
            "fixed_node_set": ";".join(nodes),
            "registry_node": nodes[0],
            "replica_nodes": ";".join(worker_nodes),
            "replicas": REPLICAS,
            "operations": OPERATIONS,
            "write_percentage": percentage,
            "failure_scenario": scenario.failure_scenario,
            "failed_node": scenario.failed_node,
            "failure_injection": "heartbeat-stop-after-half-local-operations",
            "surviving_replicas": REPLICAS - 1,
            "response_time_avg_us": f"{statistics.fmean(response_values):.9f}",
            "throughput_min_ops_per_us": f"{minimum_throughput:.9f}",
            "throughput_min_ops_per_s": f"{minimum_throughput * 1_000_000:.3f}",
            "issued_operations_total": issued_total,
            "workload_digest": workload_digest,
            "state_digest": response_digest,
            "measurement_runs": "combined" if scenario.kind == "CRDT" else "split",
            "status": "valid",
        }
    )
    for index, node in enumerate(response_survivors):
        row[f"response_node{node}_us"] = f"{response_values[index]:.9f}"
        row[f"throughput_node{node}_ops_per_us"] = (
            f"{throughput_values[index]:.9f}"
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


def row_key(row: dict[str, object]) -> tuple[str, int]:
    return str(row["failure_scenario"]), int(row["write_percentage"])


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--operations", type=int, default=OPERATIONS)
    parser.add_argument("--replicas", type=int, default=REPLICAS)
    parser.add_argument("--percentage", nargs="+", type=int, default=PERCENTAGES)
    parser.add_argument("--attempts", type=int, default=2)
    parser.add_argument("--timeout", type=int, default=1800)
    parser.add_argument(
        "--output",
        type=Path,
        default=ROOT / "results" / "hamband_infiniband_aces_4m_failures.csv",
    )
    parser.add_argument(
        "--log-dir",
        type=Path,
        default=ROOT / "results" / "hamband_infiniband_aces_4m_failure_logs",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.operations != OPERATIONS or args.replicas != REPLICAS:
        raise RuntimeError("Failure matrix requires four replicas and 4,000,000 operations")
    percentages = tuple(args.percentage)
    if set(percentages) != set(PERCENTAGES) or len(percentages) != len(PERCENTAGES):
        raise RuntimeError("Failure matrix requires exactly 15%, 20%, and 25% updates")
    if args.attempts <= 0 or args.timeout <= 0:
        raise RuntimeError("Attempts and timeout must be positive")

    nodes = allocation_nodes()
    registry_node = nodes[0]
    worker_nodes = nodes[1:]
    commit = git_commit()
    hardware = verify_allocation(worker_nodes)
    print(f"[matrix] commit={commit} hardware={hardware}", flush=True)
    print(
        f"[matrix] registry={registry_node} workers={','.join(worker_nodes)}",
        flush=True,
    )
    baselines = load_baselines()
    rows: list[dict[str, object]] = [
        dict(row) for row in load_rows(args.output) if row.get("status") == "valid"
    ]
    completed = {row_key(row) for row in rows}
    matrix = [
        (scenario, percentage)
        for scenario in SCENARIOS
        for percentage in percentages
        if (scenario.failure_scenario, percentage) not in completed
    ]
    job_id = os.environ.get("SLURM_JOB_ID", "unknown")
    port = 26000 + int(job_id) % 10000
    registry = Registry(registry_node, registry_address(registry_node), port)
    try:
        for index, (scenario, percentage) in enumerate(matrix, start=1):
            label = f"[{index}/{len(matrix)}] {scenario.paper_name} w{percentage}"
            print(f"{label}: generating ordinary baseline workload", flush=True)
            directory, workload_digest = generate_workload(scenario, percentage)
            baseline = baselines[(scenario.usecase, percentage)]
            if workload_digest != baseline["workload_digest"]:
                raise RuntimeError(
                    f"{label}: workload digest differs from ordinary baseline: "
                    f"{workload_digest} != {baseline['workload_digest']}"
                )
            try:
                last_error: Exception | None = None
                for attempt in range(1, args.attempts + 1):
                    attempt_logs = args.log_dir / f"attempt-{attempt}"
                    try:
                        if scenario.kind == "CRDT":
                            outputs = run_replicas(
                                scenario,
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
                                scenario,
                                percentage,
                                "response",
                                worker_nodes,
                                registry,
                                attempt_logs,
                                args.timeout,
                            )
                            throughput_outputs = run_replicas(
                                scenario,
                                percentage,
                                "throughput",
                                worker_nodes,
                                registry,
                                attempt_logs,
                                args.timeout,
                            )
                        row = make_row(
                            scenario,
                            percentage,
                            response_outputs,
                            throughput_outputs,
                            workload_digest,
                            baseline["state_digest"],
                            commit,
                            job_id,
                            nodes,
                            worker_nodes,
                        )
                        rows.append(row)
                        rows.sort(key=row_key)
                        save_rows(args.output, rows)
                        print(
                            f"{label}: valid response={row['response_time_avg_us']} us "
                            f"throughput={row['throughput_min_ops_per_us']} ops/us",
                            flush=True,
                        )
                        last_error = None
                        break
                    except Exception as error:
                        last_error = error
                        registry.stop()
                        print(
                            f"{label}: attempt {attempt}/{args.attempts} failed: {error}",
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

    if len(rows) != 9 or len({row_key(row) for row in rows}) != 9:
        raise RuntimeError(f"Expected nine unique failure rows, found {len(rows)}")
    print(f"[matrix] completed nine rows in {args.output}", flush=True)
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as error:
        print(f"ERROR: {error}", flush=True)
        raise

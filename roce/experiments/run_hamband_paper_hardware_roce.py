#!/usr/bin/env python3
"""Run the SafarDB Hamband matrix over OCT's hardware RoCE fabric."""

from __future__ import annotations

import argparse
import csv
import os
import re
import shlex
import statistics
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor, as_completed
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

REPO_ROOT = Path(__file__).resolve().parents[1]
REPOSITORY_ROOT = REPO_ROOT.parent


def local_commit() -> str:
    command = [
        "git", "-C", str(REPOSITORY_ROOT), "rev-parse", "--short", "HEAD"
    ]
    result = subprocess.run(
        command, capture_output=True, text=True, check=False
    )
    if result.returncode != 0:
        worktree_git_dir = (
            REPOSITORY_ROOT.parent
            / "Hamband"
            / ".git"
            / "worktrees"
            / REPOSITORY_ROOT.name
        )
        command = [
            "git",
            f"--git-dir={worktree_git_dir}",
            f"--work-tree={REPOSITORY_ROOT}",
            "rev-parse",
            "--short",
            "HEAD",
        ]
        result = subprocess.run(
            command, capture_output=True, text=True, check=False
        )
    if result.returncode != 0:
        raise RuntimeError(
            "Cannot determine the local commit; set HAMBAND_COMMIT explicitly"
        )
    return result.stdout.strip()


OPERATIONS = 4_000_000
BASE_PERCENTAGES = (0, 15, 20, 25)
EXTRA_PERCENTAGES = (5, 50)
REPLICA_COUNTS = (3, 4)
COMMIT = os.environ.get("HAMBAND_COMMIT") or local_commit()
REMOTE_ROOT = os.environ.get("HAMBAND_REMOTE_ROOT", "/users/jsaber/Hamband/roce")
REGISTRY_IP = os.environ.get("HAMBAND_REGISTRY_IP", "198.22.255.171")
RDMA_DEVICE = os.environ.get("HAMBAND_RDMA_DEVICE", "mlx5_0")
GID_INDEX = int(os.environ.get("HAMBAND_GID_INDEX", "3"))
DEFAULT_HOSTS = (
    "jsaber@pc160.cloudlab.umass.edu",
    "jsaber@pc161.cloudlab.umass.edu",
    "jsaber@pc162.cloudlab.umass.edu",
    "jsaber@pc163.cloudlab.umass.edu",
)
HOSTS = tuple(
    host.strip()
    for host in os.environ.get("HAMBAND_HOSTS", ",".join(DEFAULT_HOSTS)).split(",")
    if host.strip()
)


@dataclass(frozen=True)
class Workload:
    figure: int
    paper_name: str
    usecase: str
    kind: str
    generator: str


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

CSV_FIELDS = (
    "paper_figure",
    "paper_workload",
    "repo_usecase",
    "rdt_kind",
    "transport",
    "commit",
    "replicas",
    "operations",
    "write_percentage",
    "response_time_avg_us",
    "throughput_min_ops_per_us",
    "response_node1_us",
    "response_node2_us",
    "response_node3_us",
    "response_node4_us",
    "throughput_node1_ops_per_us",
    "throughput_node2_ops_per_us",
    "throughput_node3_ops_per_us",
    "throughput_node4_ops_per_us",
    "issued_operations_total",
    "state_digest",
    "measurement_runs",
    "status",
)

BAD_LOG_PATTERNS = (
    re.compile(r"segmentation fault", re.IGNORECASE),
    re.compile(r"terminate called", re.IGNORECASE),
    re.compile(r"what\(\):", re.IGNORECASE),
    re.compile(r"core dumped", re.IGNORECASE),
    re.compile(r"\baborted\b", re.IGNORECASE),
    re.compile(r"WRDT request failed", re.IGNORECASE),
    re.compile(r"not permissible, dropping", re.IGNORECASE),
    re.compile(r"integrity drops:\s*[1-9]", re.IGNORECASE),
    re.compile(r"dory errors:\s*[1-9]", re.IGNORECASE),
    re.compile(r"malformed method-call", re.IGNORECASE),
    re.compile(r"failed to (post|poll|set|open|query|create|connect)", re.IGNORECASE),
)

RESULTS_DIR = REPO_ROOT / "results"
CSV_PATH = RESULTS_DIR / "hamband_hardware_roce_paper_4m.csv"
LOG_DIR = RESULTS_DIR / "hamband_hardware_roce_paper_4m_logs"


def ssh_run(host: str, script: str, timeout: int) -> subprocess.CompletedProcess[str]:
    remote_command = "bash -lc " + shlex.quote(script)
    return subprocess.run(
        [
            "ssh",
            "-o",
            "BatchMode=yes",
            "-o",
            "ServerAliveInterval=30",
            "-o",
            "ServerAliveCountMax=20",
            host,
            remote_command,
        ],
        capture_output=True,
        text=True,
        timeout=timeout,
        check=False,
    )


def parallel_ssh(
    hosts: Iterable[str], scripts: Iterable[str], timeout: int
) -> list[subprocess.CompletedProcess[str]]:
    host_list = list(hosts)
    script_list = list(scripts)
    results: list[subprocess.CompletedProcess[str] | None] = [None] * len(host_list)
    with ThreadPoolExecutor(max_workers=len(host_list)) as executor:
        futures = {
            executor.submit(ssh_run, host, script, timeout): index
            for index, (host, script) in enumerate(zip(host_list, script_list))
        }
        for future in as_completed(futures):
            results[futures[future]] = future.result()
    return [result for result in results if result is not None]


def require_success(
    results: list[subprocess.CompletedProcess[str]], context: str
) -> None:
    failures = [
        f"node {index + 1}: rc={result.returncode}\n{result.stdout}\n{result.stderr}"
        for index, result in enumerate(results)
        if result.returncode != 0
    ]
    if failures:
        raise RuntimeError(context + " failed:\n" + "\n".join(failures))


def verify_cluster() -> None:
    script = f"""
set -e
test "$(git -C "$HOME/Hamband" rev-parse --short HEAD)" = "{COMMIT}"
test -x "{REMOTE_ROOT}/wellcoordination/build/bin/band"
test -x "{REMOTE_ROOT}/wellcoordination/build/bin/band-crdt"
rdma link show | grep -q "{RDMA_DEVICE}/1 state ACTIVE"
test "$(basename "$(readlink -f /sys/class/infiniband/{RDMA_DEVICE}/device/driver)")" = "mlx5_core"
test "$(cat /sys/class/infiniband/{RDMA_DEVICE}/ports/1/gid_attrs/types/{GID_INDEX})" = "RoCE v2"
grep -Eq '(^|:)ffff:' /sys/class/infiniband/{RDMA_DEVICE}/ports/1/gids/{GID_INDEX}
ibv_devinfo -d {RDMA_DEVICE} -i 1 | grep -q 'state:[[:space:]]*PORT_ACTIVE'
ibv_devinfo -d {RDMA_DEVICE} -i 1 | grep -q 'link_layer:[[:space:]]*Ethernet'
"""
    results = parallel_ssh(HOSTS, [script] * len(HOSTS), 30)
    require_success(results, "cluster verification")


def generate_workload(workload: Workload, replicas: int, percentage: int) -> None:
    hosts = HOSTS[:replicas]
    writes = OPERATIONS * percentage // 100
    config = f"{replicas}-{OPERATIONS}-{percentage}"
    scripts = []
    for _ in hosts:
        scripts.append(
            f"""
set -e
ROOT="{REMOTE_ROOT}"
WORKLOAD="$ROOT/wellcoordination/workload"
DIR="$WORKLOAD/{config}/{workload.usecase}"
rm -rf "$DIR"
HAMBAND_WORKLOAD_DIR="$WORKLOAD" \
  "$ROOT/wellcoordination/build/bin/{workload.generator}" \
  {replicas} {OPERATIONS} {percentage}
test "$(find "$DIR" -maxdepth 1 -type f -name '*.txt' | wc -l)" -eq {replicas}
test "$(grep -h -v '^#' "$DIR"/*.txt | grep -c '.')" -eq {OPERATIONS}
test "$(grep -h '^#' "$DIR"/*.txt | sort -u)" = "#{writes}"
sha256sum "$DIR"/*.txt
"""
        )
    results = parallel_ssh(hosts, scripts, 600)
    require_success(results, f"workload generation for {config}/{workload.usecase}")

    hashes = []
    for result in results:
        node_hashes = [
            line.split()[0]
            for line in result.stdout.splitlines()
            if re.fullmatch(r"[0-9a-f]{64}\s+.+\.txt", line)
        ]
        if len(node_hashes) != replicas:
            raise RuntimeError("Could not verify every generated workload file")
        hashes.append(node_hashes)
    if any(node_hashes != hashes[0] for node_hashes in hashes[1:]):
        raise RuntimeError("Generated workload files differ between replicas")


def stop_experiment_processes() -> None:
    script = """
pkill -u "$USER" -x band 2>/dev/null || true
pkill -u "$USER" -x band-crdt 2>/dev/null || true
pkill -u "$USER" -x timeout 2>/dev/null || true
"""
    parallel_ssh(HOSTS, [script] * len(HOSTS), 30)


def restart_registry() -> None:
    stop_experiment_processes()
    script = f"""
set -e
pkill -u "$USER" -x memcached 2>/dev/null || true
rm -f /tmp/hamband-memcached.pid /tmp/hamband-memcached.log
for attempt in $(seq 1 10); do
  nohup memcached -l {REGISTRY_IP} -p 9999 -U 0 -m 64 \
    > /tmp/hamband-memcached.log 2>&1 < /dev/null &
  echo $! > /tmp/hamband-memcached.pid
  sleep 1
  if kill -0 "$(cat /tmp/hamband-memcached.pid)" 2>/dev/null; then
    break
  fi
done
kill -0 "$(cat /tmp/hamband-memcached.pid)"
ss -ltn | grep -q "{REGISTRY_IP}:9999"
"""
    result = ssh_run(HOSTS[0], script, 30)
    require_success([result], "registry restart")


def run_replicas(
    workload: Workload,
    replicas: int,
    percentage: int,
    mode: str,
    timeout: int,
) -> list[str]:
    restart_registry()
    hosts = HOSTS[:replicas]
    scripts = []
    for node_id in range(1, replicas + 1):
        if workload.kind == "CRDT":
            invocation = (
                f'"$ROOT/wellcoordination/build/bin/band-crdt" '
                f"{node_id} {replicas} {OPERATIONS} {percentage} "
                f"{workload.usecase} 0"
            )
        else:
            throughput_flag = 1 if mode == "throughput" else 0
            invocation = (
                f'"$ROOT/wellcoordination/build/bin/band" '
                f"{node_id} {replicas} {OPERATIONS} {percentage} "
                f"{workload.usecase} {throughput_flag} 0"
            )
        scripts.append(
            f"""
set -e
ROOT="{REMOTE_ROOT}"
export DORY_RDMA_DEVICE={RDMA_DEVICE}
export DORY_GID_INDEX={GID_INDEX}
export DORY_REGISTRY_IP={REGISTRY_IP}:9999
export HAMBAND_WORKLOAD_DIR="$ROOT/wellcoordination/workload"
timeout --signal=TERM --kill-after=10s {timeout}s {invocation}
"""
        )

    results = parallel_ssh(hosts, scripts, timeout + 60)
    run_name = (
        f"fig{workload.figure}-{workload.usecase}-r{replicas}-"
        f"w{percentage}-{mode}"
    )
    run_dir = LOG_DIR / run_name
    run_dir.mkdir(parents=True, exist_ok=True)
    for node_id, result in enumerate(results, start=1):
        output = result.stdout
        if result.stderr:
            output += "\n[stderr]\n" + result.stderr
        (run_dir / f"node{node_id}.log").write_text(output, encoding="utf-8")

    require_success(results, run_name)
    outputs = [result.stdout + "\n" + result.stderr for result in results]
    validate_logs(outputs, replicas, run_name)
    return outputs


def extract_values(outputs: list[str], pattern: str, label: str) -> list[float]:
    values = []
    compiled = re.compile(pattern, re.MULTILINE)
    for node_id, output in enumerate(outputs, start=1):
        matches = compiled.findall(output)
        if len(matches) != 1:
            raise RuntimeError(
                f"Expected one {label} value on node {node_id}, found {len(matches)}"
            )
        values.append(float(matches[0]))
    return values


def validate_logs(outputs: list[str], replicas: int, run_name: str) -> None:
    issued = []
    digests = []
    for node_id, output in enumerate(outputs, start=1):
        if output.count("all nodes finished") != 1:
            raise RuntimeError(f"{run_name}: node {node_id} missed finish barrier")
        for pattern in BAD_LOG_PATTERNS:
            if pattern.search(output):
                raise RuntimeError(
                    f"{run_name}: node {node_id} matched error pattern {pattern.pattern}"
                )
        issued_matches = re.findall(r"^issued\s+(\d+)\s+operations$", output, re.MULTILINE)
        digest_matches = re.findall(r"^state digest:\s*(\d+)\s*$", output, re.MULTILINE)
        if len(issued_matches) != 1 or len(digest_matches) != 1:
            raise RuntimeError(f"{run_name}: node {node_id} has incomplete metrics")
        issued.append(int(issued_matches[0]))
        digests.append(digest_matches[0])

    if len(outputs) != replicas or sum(issued) != OPERATIONS:
        raise RuntimeError(
            f"{run_name}: issued {sum(issued)} operations across {len(outputs)} nodes"
        )
    if len(set(digests)) != 1:
        raise RuntimeError(f"{run_name}: replicas did not converge: {digests}")


def clean_workload(workload: Workload, replicas: int, percentage: int) -> None:
    directory = (
        f"{REMOTE_ROOT}/wellcoordination/workload/"
        f"{replicas}-{OPERATIONS}-{percentage}/{workload.usecase}"
    )
    script = f'rm -rf "{directory}"'
    parallel_ssh(HOSTS[:replicas], [script] * replicas, 60)


def load_rows() -> list[dict[str, str]]:
    if not CSV_PATH.exists():
        return []
    with CSV_PATH.open(newline="", encoding="utf-8") as source:
        return list(csv.DictReader(source))


def save_rows(rows: list[dict[str, object]]) -> None:
    RESULTS_DIR.mkdir(parents=True, exist_ok=True)
    temporary = CSV_PATH.with_suffix(".csv.tmp")
    with temporary.open("w", newline="", encoding="utf-8") as destination:
        writer = csv.DictWriter(destination, fieldnames=CSV_FIELDS)
        writer.writeheader()
        writer.writerows(rows)
    temporary.replace(CSV_PATH)


def row_key(row: dict[str, object]) -> tuple[str, int, int]:
    return (
        str(row["repo_usecase"]),
        int(row["replicas"]),
        int(row["write_percentage"]),
    )


def make_row(
    workload: Workload,
    replicas: int,
    percentage: int,
    response_outputs: list[str],
    throughput_outputs: list[str],
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
    response_digests = re.findall(
        r"^state digest:\s*(\d+)\s*$", response_outputs[0], re.MULTILINE
    )
    throughput_digests = re.findall(
        r"^state digest:\s*(\d+)\s*$", throughput_outputs[0], re.MULTILINE
    )
    if len(response_digests) != 1 or len(throughput_digests) != 1:
        raise RuntimeError("Cannot extract validated response/throughput state digests")
    if response_digests[0] != throughput_digests[0]:
        raise RuntimeError("Response and throughput runs reached different final states")
    issued_total = sum(
        int(value)
        for output in response_outputs
        for value in re.findall(
            r"^issued\s+(\d+)\s+operations$", output, re.MULTILINE
        )
    )

    row: dict[str, object] = {
        "paper_figure": workload.figure,
        "paper_workload": workload.paper_name,
        "repo_usecase": workload.usecase,
        "rdt_kind": workload.kind,
        "transport": "RoCEv2-Hardware-mlx5_0",
        "commit": COMMIT,
        "replicas": replicas,
        "operations": OPERATIONS,
        "write_percentage": percentage,
        "response_time_avg_us": f"{statistics.fmean(response_values):.9f}",
        "throughput_min_ops_per_us": f"{min(throughput_values):.9f}",
        "issued_operations_total": issued_total,
        "state_digest": response_digests[0],
        "measurement_runs": "combined" if workload.kind == "CRDT" else "split",
        "status": "valid",
    }
    for node in range(1, 5):
        row[f"response_node{node}_us"] = (
            f"{response_values[node - 1]:.9f}" if node <= replicas else ""
        )
        row[f"throughput_node{node}_ops_per_us"] = (
            f"{throughput_values[node - 1]:.9f}" if node <= replicas else ""
        )
    return row


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--workload", choices=[item.usecase for item in WORKLOADS])
    parser.add_argument("--replicas", type=int, choices=REPLICA_COUNTS)
    parser.add_argument(
        "--percentage",
        type=int,
        choices=tuple(sorted({*BASE_PERCENTAGES, *EXTRA_PERCENTAGES})),
    )
    parser.add_argument("--reuse-commit", action="append", default=[])
    parser.add_argument("--timeout", type=int, default=3600)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    RESULTS_DIR.mkdir(parents=True, exist_ok=True)
    LOG_DIR.mkdir(parents=True, exist_ok=True)
    verify_cluster()

    accepted_commits = {COMMIT, *args.reuse_commit}
    rows: list[dict[str, object]] = [
        dict(row)
        for row in load_rows()
        if row.get("commit") in accepted_commits and row.get("status") == "valid"
    ]
    completed = {row_key(row) for row in rows}
    selected_workloads = [
        workload
        for workload in WORKLOADS
        if args.workload is None or workload.usecase == args.workload
    ]
    selected_replicas = (
        (args.replicas,) if args.replicas is not None else REPLICA_COUNTS
    )
    if max(selected_replicas) > len(HOSTS):
        raise RuntimeError(
            f"Requested {max(selected_replicas)} replicas but only {len(HOSTS)} hosts "
            "are configured"
        )
    matrix = [
        (workload, replicas, percentage)
        for workload in selected_workloads
        for replicas in selected_replicas
        for percentage in (
            (args.percentage,)
            if args.percentage is not None
            else BASE_PERCENTAGES
            + (EXTRA_PERCENTAGES if workload.usecase in {"kvstore", "smallbank"} else ())
        )
    ]
    pending = [
        item
        for item in matrix
        if (item[0].usecase, item[1], item[2]) not in completed
    ]
    print(
        f"Starting {len(pending)} pending configurations "
        f"({len(matrix) - len(pending)} already complete)",
        flush=True,
    )

    for index, (workload, replicas, percentage) in enumerate(pending, start=1):
        label = (
            f"[{index}/{len(pending)}] Figure {workload.figure} "
            f"{workload.paper_name}, {replicas} replicas, {percentage}% writes"
        )
        print(label + ": generating", flush=True)
        generate_workload(workload, replicas, percentage)

        if workload.kind == "CRDT":
            print(label + ": combined response/throughput run", flush=True)
            outputs = run_replicas(
                workload, replicas, percentage, "combined", args.timeout
            )
            response_outputs = outputs
            throughput_outputs = outputs
        else:
            print(label + ": response-time run", flush=True)
            response_outputs = run_replicas(
                workload, replicas, percentage, "response", args.timeout
            )
            print(label + ": throughput run", flush=True)
            throughput_outputs = run_replicas(
                workload, replicas, percentage, "throughput", args.timeout
            )

        row = make_row(
            workload,
            replicas,
            percentage,
            response_outputs,
            throughput_outputs,
        )
        rows.append(row)
        rows.sort(key=lambda item: (int(item["paper_figure"]), str(item["repo_usecase"]),
                                    int(item["replicas"]), int(item["write_percentage"])))
        save_rows(rows)
        clean_workload(workload, replicas, percentage)
        print(
            label
            + f": valid, response={row['response_time_avg_us']} us, "
            + f"throughput={row['throughput_min_ops_per_us']} ops/us",
            flush=True,
        )

    stop_experiment_processes()
    print(f"Completed. CSV: {CSV_PATH}", flush=True)
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as error:
        print(f"ERROR: {error}", file=sys.stderr, flush=True)
        stop_experiment_processes()
        raise

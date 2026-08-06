#!/usr/bin/env python3
"""Independently audit the completed ACES Hamband paper matrix."""

from __future__ import annotations

import argparse
import csv
import math
import re
import statistics
from dataclasses import dataclass
from pathlib import Path


OPERATIONS = 4_000_000
REPLICAS = tuple(range(3, 9))
BASE_PERCENTAGES = (0, 15, 20, 25)
EXTRA_PERCENTAGES = (5, 50)


@dataclass(frozen=True)
class Workload:
    figure: int
    paper_name: str
    usecase: str
    kind: str

    @property
    def percentages(self) -> tuple[int, ...]:
        if self.usecase in {"kvstore", "smallbank"}:
            return tuple(sorted((*BASE_PERCENTAGES, *EXTRA_PERCENTAGES)))
        return BASE_PERCENTAGES


WORKLOADS = (
    Workload(9, "Counter", "counter", "CRDT"),
    Workload(9, "Register", "register", "CRDT"),
    Workload(9, "G-Set", "gset", "CRDT"),
    Workload(9, "PN-Set", "pnset", "CRDT"),
    Workload(9, "2P-Set", "twopset", "CRDT"),
    Workload(10, "Account", "account", "WRDT"),
    Workload(10, "Courseware", "courseware", "WRDT"),
    Workload(10, "Project", "project", "WRDT"),
    Workload(10, "Movie", "movie", "WRDT"),
    Workload(10, "Auction", "rubis", "WRDT"),
    Workload(11, "YCSB", "kvstore", "CRDT"),
    Workload(11, "SmallBank", "smallbank", "WRDT"),
)
WORKLOAD_BY_USECASE = {workload.usecase: workload for workload in WORKLOADS}

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

RUN_DIRECTORY = re.compile(
    r"^fig(?P<figure>\d+)-(?P<usecase>[a-z0-9]+)-r(?P<replicas>\d+)"
    r"-w(?P<percentage>\d+)-(?P<mode>combined|response|throughput)$"
)
ISSUED = re.compile(r"^issued\s+(\d+)\s+operations$", re.MULTILINE)
STATE_DIGEST = re.compile(r"^state digest:\s*(\d+)\s*$", re.MULTILINE)
RESPONSE = re.compile(
    r"^total average response time for \d+ calls:\s*([0-9.eE+-]+)\s*$",
    re.MULTILINE,
)
THROUGHPUT = re.compile(r"^throughput:\s*([0-9.eE+-]+)\s*$", re.MULTILINE)


def require(condition: bool, message: str) -> None:
    if not condition:
        raise RuntimeError(message)


def positive_float(value: str, label: str) -> float:
    try:
        number = float(value)
    except ValueError as error:
        raise RuntimeError(f"{label} is not numeric: {value!r}") from error
    require(math.isfinite(number) and number > 0, f"{label} must be finite and positive")
    return number


def expected_keys() -> set[tuple[str, int, int]]:
    return {
        (workload.usecase, replicas, percentage)
        for workload in WORKLOADS
        for replicas in REPLICAS
        for percentage in workload.percentages
    }


def expected_run_directories() -> dict[str, tuple[str, int, int, str]]:
    expected: dict[str, tuple[str, int, int, str]] = {}
    for workload in WORKLOADS:
        for replicas in REPLICAS:
            for percentage in workload.percentages:
                modes = ("combined",) if workload.kind == "CRDT" else (
                    "response",
                    "throughput",
                )
                for mode in modes:
                    name = (
                        f"fig{workload.figure}-{workload.usecase}-r{replicas}"
                        f"-w{percentage}-{mode}"
                    )
                    expected[name] = (workload.usecase, replicas, percentage, mode)
    return expected


def audit_csv(
    path: Path,
    commit: str,
    job_id: str,
    nodes: tuple[str, ...],
) -> dict[tuple[str, int, int], dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as source:
        reader = csv.DictReader(source)
        require(tuple(reader.fieldnames or ()) == CSV_FIELDS, "CSV schema/order is unexpected")
        rows = list(reader)

    expected = expected_keys()
    require(len(expected) == 312, "Independent expected matrix is not 312 rows")
    require(len(rows) == len(expected), f"Expected 312 CSV rows, found {len(rows)}")

    fixed_node_set = ";".join(nodes)
    registry_node = nodes[0]
    worker_nodes = nodes[1:]
    by_key: dict[tuple[str, int, int], dict[str, str]] = {}

    for line_number, row in enumerate(rows, start=2):
        label = f"CSV line {line_number}"
        try:
            replicas = int(row["replicas"])
            percentage = int(row["write_percentage"])
        except ValueError as error:
            raise RuntimeError(f"{label} has invalid integer dimensions") from error
        key = (row["repo_usecase"], replicas, percentage)
        require(key in expected, f"{label} has unexpected key {key}")
        require(key not in by_key, f"Duplicate CSV key {key}")
        by_key[key] = row

        workload = WORKLOAD_BY_USECASE[key[0]]
        require(row["paper_figure"] == str(workload.figure), f"{label} figure mismatch")
        require(row["paper_workload"] == workload.paper_name, f"{label} workload mismatch")
        require(row["rdt_kind"] == workload.kind, f"{label} RDT kind mismatch")
        require(row["transport"] == "InfiniBand", f"{label} transport mismatch")
        require(row["commit"] == commit, f"{label} commit mismatch")
        require(row["slurm_job_id"] == job_id, f"{label} job id mismatch")
        require(row["fixed_node_set"] == fixed_node_set, f"{label} fixed nodes mismatch")
        require(row["registry_node"] == registry_node, f"{label} registry mismatch")
        require(
            row["replica_nodes"] == ";".join(worker_nodes[:replicas]),
            f"{label} replica prefix mismatch",
        )
        require(row["operations"] == str(OPERATIONS), f"{label} operation count mismatch")
        require(
            row["issued_operations_total"] == str(OPERATIONS),
            f"{label} issued operation count mismatch",
        )
        require(row["status"] == "valid", f"{label} status is not valid")
        require(
            row["measurement_runs"] == ("combined" if workload.kind == "CRDT" else "split"),
            f"{label} measurement mode mismatch",
        )
        require(
            re.fullmatch(r"[0-9a-f]{64}", row["workload_digest"]) is not None,
            f"{label} workload digest is invalid",
        )
        require(
            re.fullmatch(r"\d+", row["state_digest"]) is not None,
            f"{label} state digest is invalid",
        )

        response_values: list[float] = []
        throughput_values: list[float] = []
        for node in range(1, 9):
            response = row[f"response_node{node}_us"]
            throughput = row[f"throughput_node{node}_ops_per_us"]
            if node <= replicas:
                response_values.append(positive_float(response, f"{label} node {node} response"))
                throughput_values.append(
                    positive_float(throughput, f"{label} node {node} throughput")
                )
            else:
                require(not response, f"{label} has response data for inactive node {node}")
                require(not throughput, f"{label} has throughput data for inactive node {node}")

        response_average = positive_float(row["response_time_avg_us"], f"{label} response")
        throughput_minimum = positive_float(
            row["throughput_min_ops_per_us"], f"{label} throughput"
        )
        throughput_per_second = positive_float(
            row["throughput_min_ops_per_s"], f"{label} throughput per second"
        )
        require(
            math.isclose(
                response_average,
                statistics.fmean(response_values),
                rel_tol=0,
                abs_tol=2e-9,
            ),
            f"{label} response average was not computed from its replica values",
        )
        require(
            math.isclose(
                throughput_minimum,
                min(throughput_values),
                rel_tol=0,
                abs_tol=2e-9,
            ),
            f"{label} throughput is not the replica minimum",
        )
        require(
            math.isclose(
                throughput_per_second,
                throughput_minimum * 1_000_000,
                rel_tol=0,
                abs_tol=0.0011,
            ),
            f"{label} throughput unit conversion mismatch",
        )

    require(set(by_key) == expected, "CSV key set does not equal the expected matrix")
    return by_key


def one_metric(pattern: re.Pattern[str], output: str, label: str) -> float:
    matches = pattern.findall(output)
    require(len(matches) == 1, f"{label}: expected one metric, found {len(matches)}")
    return positive_float(matches[0], label)


def audit_logs(
    log_root: Path,
    csv_rows: dict[tuple[str, int, int], dict[str, str]],
) -> tuple[int, int]:
    attempt_one = log_root / "attempt-1"
    require(attempt_one.is_dir(), f"Missing log directory {attempt_one}")
    attempt_two = log_root / "attempt-2"
    require(not attempt_two.exists(), "Attempt-2 logs exist despite a zero-failure run")

    expected_directories = expected_run_directories()
    actual_directories = {path.name: path for path in attempt_one.iterdir() if path.is_dir()}
    require(
        set(actual_directories) == set(expected_directories),
        "Run log directory set does not match the expected 468 executions",
    )

    run_data: dict[tuple[str, int, int, str], tuple[str, list[float], list[float]]] = {}
    node_log_count = 0
    for name, expected in expected_directories.items():
        usecase, replicas, percentage, mode = expected
        run_dir = actual_directories[name]
        match = RUN_DIRECTORY.fullmatch(name)
        require(match is not None, f"Malformed run directory name {name}")
        require(int(match["figure"]) == WORKLOAD_BY_USECASE[usecase].figure, f"{name}: figure mismatch")
        require(match["usecase"] == usecase, f"{name}: usecase mismatch")
        require(int(match["replicas"]) == replicas, f"{name}: replica mismatch")
        require(int(match["percentage"]) == percentage, f"{name}: percentage mismatch")
        require(match["mode"] == mode, f"{name}: mode mismatch")
        require((run_dir / "registry.log").is_file(), f"{name}: missing registry log")

        expected_node_logs = [run_dir / f"node{node}.log" for node in range(1, replicas + 1)]
        actual_node_logs = sorted(run_dir.glob("node*.log"))
        require(actual_node_logs == expected_node_logs, f"{name}: node log set mismatch")
        node_log_count += len(actual_node_logs)

        issued_total = 0
        digests: list[str] = []
        responses: list[float] = []
        throughputs: list[float] = []
        for node, path in enumerate(actual_node_logs, start=1):
            output = path.read_text(encoding="utf-8", errors="replace")
            label = f"{name}/node{node}"
            require(output.count("all nodes finished") == 1, f"{label}: finish barrier mismatch")
            for pattern in BAD_LOG_PATTERNS:
                require(pattern.search(output) is None, f"{label}: matched {pattern.pattern}")
            issued = ISSUED.findall(output)
            digest = STATE_DIGEST.findall(output)
            require(len(issued) == 1, f"{label}: issued metric mismatch")
            require(len(digest) == 1, f"{label}: state digest mismatch")
            issued_total += int(issued[0])
            digests.append(digest[0])
            if mode in {"combined", "response"}:
                responses.append(one_metric(RESPONSE, output, f"{label} response"))
            if mode in {"combined", "throughput"}:
                throughputs.append(one_metric(THROUGHPUT, output, f"{label} throughput"))

        require(issued_total == OPERATIONS, f"{name}: issued {issued_total} operations")
        require(len(set(digests)) == 1, f"{name}: replicas did not converge")
        csv_row = csv_rows[(usecase, replicas, percentage)]
        require(digests[0] == csv_row["state_digest"], f"{name}: CSV digest mismatch")
        run_data[(usecase, replicas, percentage, mode)] = (
            digests[0],
            responses,
            throughputs,
        )

    for key, row in csv_rows.items():
        usecase, replicas, percentage = key
        workload = WORKLOAD_BY_USECASE[usecase]
        if workload.kind == "CRDT":
            digest, responses, throughputs = run_data[(*key, "combined")]
        else:
            response_digest, responses, _ = run_data[(*key, "response")]
            throughput_digest, _, throughputs = run_data[(*key, "throughput")]
            require(response_digest == throughput_digest, f"{key}: split-run digest mismatch")
            digest = response_digest
        require(digest == row["state_digest"], f"{key}: final digest mismatch")
        require(len(responses) == replicas, f"{key}: response count mismatch")
        require(len(throughputs) == replicas, f"{key}: throughput count mismatch")
        for node in range(1, replicas + 1):
            require(
                math.isclose(
                    responses[node - 1],
                    float(row[f"response_node{node}_us"]),
                    rel_tol=0,
                    abs_tol=5e-10,
                ),
                f"{key}: node {node} response differs from raw log",
            )
            require(
                math.isclose(
                    throughputs[node - 1],
                    float(row[f"throughput_node{node}_ops_per_us"]),
                    rel_tol=0,
                    abs_tol=5e-10,
                ),
                f"{key}: node {node} throughput differs from raw log",
            )

    return len(actual_directories), node_log_count


def audit_job_log(path: Path, nodes: tuple[str, ...]) -> None:
    output = path.read_text(encoding="utf-8", errors="replace")
    require("[matrix] completed 312 valid rows" in output, "Job log lacks matrix completion")
    require(re.search(r"^completed=", output, re.MULTILINE) is not None, "Job log lacks completion timestamp")
    require(re.search(r"attempt \d+/\d+ failed", output) is None, "Job log contains a failed attempt")
    valid_rows = re.findall(r"^\[\d+/312\].*: valid response=", output, re.MULTILINE)
    require(len(valid_rows) == 312, f"Job log contains {len(valid_rows)} full-matrix valid rows")
    for node in nodes:
        marker = f"[allocation] {node} numa=2 infiniband=active"
        require(output.count(marker) == 2, f"Job log hardware checks mismatch for {node}")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--csv", type=Path, required=True)
    parser.add_argument("--log-dir", type=Path, required=True)
    parser.add_argument("--job-log", type=Path, required=True)
    parser.add_argument("--expected-commit", required=True)
    parser.add_argument("--expected-job-id", required=True)
    parser.add_argument(
        "--expected-nodes",
        required=True,
        help="Semicolon-separated registry node followed by eight worker nodes",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    nodes = tuple(args.expected_nodes.split(";"))
    require(len(nodes) == 9 and len(set(nodes)) == 9, "Expected exactly nine unique nodes")
    rows = audit_csv(args.csv, args.expected_commit, args.expected_job_id, nodes)
    run_directories, node_logs = audit_logs(args.log_dir, rows)
    audit_job_log(args.job_log, nodes)
    print("audit=PASS")
    print(f"csv_rows={len(rows)}")
    print(f"run_directories={run_directories}")
    print(f"node_logs={node_logs}")
    print(f"fixed_nodes={';'.join(nodes)}")
    print(f"commit={args.expected_commit}")
    print(f"slurm_job_id={args.expected_job_id}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

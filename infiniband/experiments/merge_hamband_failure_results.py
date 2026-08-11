#!/usr/bin/env python3
"""Validate nine ACES failure rows and merge them into the paper CSV."""

from __future__ import annotations

import argparse
import csv
import hashlib
from pathlib import Path
from typing import Sequence


FAILURE_FIELDS = (
    "failure_scenario",
    "failed_node",
    "failure_injection",
    "surviving_replicas",
)
EXPECTED = {
    ("follower-failure", percentage): ("account", "Account follower-failure", "2")
    for percentage in (15, 20, 25)
}
EXPECTED.update(
    {
        ("leader-failure", percentage): ("account", "Account leader-failure", "1")
        for percentage in (15, 20, 25)
    }
)
EXPECTED.update(
    {
        ("replica-failure", percentage): ("twopset", "2P-Set replica-failure", "1")
        for percentage in (15, 20, 25)
    }
)


def require(condition: bool, message: str) -> None:
    if not condition:
        raise RuntimeError(message)


def load(path: Path) -> tuple[list[str], list[dict[str, str]]]:
    with path.open(newline="", encoding="utf-8") as source:
        reader = csv.DictReader(source)
        require(reader.fieldnames is not None, f"Missing CSV header: {path}")
        return list(reader.fieldnames), list(reader)


def ordinary_key(row: dict[str, str]) -> tuple[str, int, int]:
    return row["repo_usecase"], int(row["replicas"]), int(row["write_percentage"])


def failure_key(row: dict[str, str]) -> tuple[str, int]:
    return row["failure_scenario"], int(row["write_percentage"])


def positive(value: str) -> bool:
    try:
        return float(value) > 0
    except (TypeError, ValueError):
        return False


def validate(
    baseline_rows: Sequence[dict[str, str]], failure_rows: Sequence[dict[str, str]]
) -> list[dict[str, str]]:
    ordinary = [
        row
        for row in baseline_rows
        if row.get("failure_scenario", "") in ("", "none")
    ]
    require(len(ordinary) == 312, f"Expected 312 ordinary rows, found {len(ordinary)}")
    ordinary_keys = [ordinary_key(row) for row in ordinary]
    require(len(set(ordinary_keys)) == 312, "Ordinary matrix contains duplicate keys")
    baselines = {
        ordinary_key(row): row
        for row in ordinary
        if row["repo_usecase"] in ("account", "twopset")
        and int(row["replicas"]) == 4
        and int(row["write_percentage"]) in (15, 20, 25)
    }
    require(len(baselines) == 6, f"Expected six matching baselines, found {len(baselines)}")

    require(len(failure_rows) == 9, f"Expected nine failure rows, found {len(failure_rows)}")
    keys = [failure_key(row) for row in failure_rows]
    require(set(keys) == set(EXPECTED), f"Failure keys differ: {sorted(keys)}")
    require(len(set(keys)) == 9, "Failure rows contain duplicate keys")
    require(len({row["commit"] for row in failure_rows}) == 1, "Failure commits differ")
    require(len({row["slurm_job_id"] for row in failure_rows}) == 1, "Failure jobs differ")
    require(len({row["fixed_node_set"] for row in failure_rows}) == 1, "Node sets differ")

    for row in failure_rows:
        key = failure_key(row)
        usecase, paper_name, failed_node = EXPECTED[key]
        require(row["paper_figure"] == "14", f"{key}: wrong paper figure")
        require(row["paper_workload"] == paper_name, f"{key}: wrong paper name")
        require(row["repo_usecase"] == usecase, f"{key}: wrong use case")
        require(row["replicas"] == "4", f"{key}: wrong replica count")
        require(row["operations"] == "4000000", f"{key}: wrong operation count")
        require(row["failed_node"] == failed_node, f"{key}: wrong failed node")
        require(row["surviving_replicas"] == "3", f"{key}: wrong survivor count")
        require(
            row["failure_injection"] == "fail-stop-after-half-local-operations",
            f"{key}: wrong failure injection",
        )
        require(row["issued_operations_total"] == "4000000", f"{key}: lost calls")
        require(row["status"] == "valid", f"{key}: invalid status")
        require(positive(row["response_time_avg_us"]), f"{key}: invalid response")
        require(positive(row["throughput_min_ops_per_us"]), f"{key}: invalid throughput")
        baseline = baselines[(usecase, 4, key[1])]
        require(
            row["workload_digest"] == baseline["workload_digest"],
            f"{key}: workload differs from baseline",
        )
        require(
            row["state_digest"] == baseline["state_digest"],
            f"{key}: final state differs from baseline",
        )
        require(
            row["measurement_runs"] == ("combined" if usecase == "twopset" else "split"),
            f"{key}: wrong measurement mode",
        )
        for node in range(1, 5):
            response = row[f"response_node{node}_us"]
            throughput = row[f"throughput_node{node}_ops_per_us"]
            if str(node) == failed_node:
                require(not response and not throughput, f"{key}: failed node has metrics")
            else:
                require(positive(response), f"{key}: missing survivor response")
                require(positive(throughput), f"{key}: missing survivor throughput")
    return ordinary


def save(path: Path, fields: Sequence[str], rows: Sequence[dict[str, str]]) -> None:
    temporary = path.with_suffix(path.suffix + ".tmp")
    with temporary.open("w", newline="", encoding="utf-8") as destination:
        writer = csv.DictWriter(destination, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)
    temporary.replace(path)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--baseline", type=Path, required=True)
    parser.add_argument("--failures", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    baseline_fields, baseline_rows = load(args.baseline)
    failure_fields, failure_rows = load(args.failures)
    ordinary = validate(baseline_rows, failure_rows)

    fields = [field for field in baseline_fields if field not in FAILURE_FIELDS]
    insertion = fields.index("write_percentage") + 1
    fields[insertion:insertion] = list(FAILURE_FIELDS)
    require(set(failure_fields) == set(fields), "Failure CSV schema differs from merged schema")

    merged: list[dict[str, str]] = []
    for row in ordinary:
        normalized = {field: row.get(field, "") for field in fields}
        normalized.update(
            {
                "failure_scenario": "none",
                "failed_node": "0",
                "failure_injection": "",
                "surviving_replicas": row["replicas"],
            }
        )
        merged.append(normalized)
    merged.extend({field: row.get(field, "") for field in fields} for row in failure_rows)
    merged.sort(
        key=lambda row: (
            int(row["paper_figure"]),
            row["repo_usecase"],
            row["failure_scenario"],
            int(row["replicas"]),
            int(row["write_percentage"]),
        )
    )
    require(len(merged) == 321, f"Expected 321 merged rows, found {len(merged)}")
    args.output.parent.mkdir(parents=True, exist_ok=True)
    save(args.output, fields, merged)
    digest = hashlib.sha256(args.output.read_bytes()).hexdigest()
    print(f"PASS rows=321 ordinary=312 failure=9 sha256={digest}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

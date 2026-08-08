# Hamband over RoCE

This document describes the RoCE transport variant of Hamband, how it is
isolated from the InfiniBand implementation, the original two-node account
smoke test, and the completed 3--4 replica paper matrix.

## Status

This `roce/` project has been built and tested on these CloudLab nodes:

- `jsaber@pc160.cloudlab.umass.edu`
- `jsaber@pc161.cloudlab.umass.edu`
- `jsaber@pc162.cloudlab.umass.edu`
- `jsaber@pc163.cloudlab.umass.edu`

The initial smoke test used two replicas, failure handling disabled, 100%
writes, and 1,000,000 total account operations. Both replicas completed and
converged to the independently verified balance `4752077`.

The completed paper matrix uses 3 and 4 replicas, 4,000,000 operations per
configuration, and all 12 Figure 9--11 workloads. Every workload has 0%, 15%,
20%, and 25% writes; YCSB and SmallBank additionally have 5% and 50% rows. The
published CSV contains all 104 expected configurations.

These are CPU Soft-RoCE results. They prove that Hamband and Mu can run through
the RoCE verbs path at up to four replicas, but they are not hardware-RoCE or
FPGA performance results.

## Protocol Preservation

The RoCE work does not change Hamband or Mu protocol semantics. In particular,
it does not change:

- leader and follower roles;
- quorum calculations or the two-node quorum support;
- operation ordering or consensus decisions;
- conflicting and non-conflicting operation classification;
- bank-account permissibility and integrity checks;
- failure behavior; or
- the benchmark operation distribution.

The changes are confined to RDMA addressing, queue completion handling, device
selection, build portability, and narrow completion-bookkeeping fixes that do
not change quorum size or protocol decisions. The InfiniBand transport variant
is maintained separately in the sibling `../infiniband/` directory.

## Keeping InfiniBand and RoCE Side by Side

Clone `master` once; both complete implementations are included:

```bash
git clone https://github.com/fhoushmand/Hamband.git
cd Hamband/roce
```

Use `../infiniband/` for the InfiniBand implementation and this `roce/`
directory for RoCE runs. Each README documents its narrow build/correctness
fixes. No source or CMake edits are needed when selecting a transport.

Do not mix binaries from `infiniband/` and `roce/` in one experiment. This
project exchanges additional GID and MTU transport metadata, so every replica
in a run must use binaries built from this same directory.

This RoCE-capable project can also use a real InfiniBand port. It detects the
selected port's link layer and chooses the addressing mode automatically:

| Selected port | Addressing used |
| --- | --- |
| InfiniBand | LID |
| Ethernet/RoCE | GID plus GRH |

Transport selection is controlled through environment variables:

```bash
# RoCE example
export DORY_RDMA_DEVICE=rxe0
export DORY_GID_INDEX=1

# InfiniBand example with the dual-capable project
export DORY_RDMA_DEVICE=mlx5_0
unset DORY_GID_INDEX
```

`DORY_GID_INDEX` is consulted only for an Ethernet/RoCE port. The correct index
depends on the host and network configuration.

## What Changed for RoCE

The minimum transport changes are:

1. Ethernet RDMA ports are accepted in addition to InfiniBand ports.
2. The local RoCE GID is queried and exchanged with each peer.
3. RoCE queue pairs use a global route header with the remote GID and local GID
   index; InfiniBand queue pairs continue to use the remote LID.
4. Peers exchange their active MTU and use the smaller active MTU when bringing
   a queue pair to RTR.
5. `DORY_RDMA_DEVICE` selects the verbs device deterministically.
6. Available broadcast completions are drained and checked for errors.
7. If Soft-RoCE temporarily fills either Hamband's broadcast queue or Mu's
   majority-write queue, the owner drains completed writes and retries the same
   write. This prevents a remote-log hole and does not alter protocol decisions.
8. Outstanding broadcast writes are flushed after request issuance, before
   final-state convergence is accepted.
9. Finite WRDT runs append an ignored ordered marker after timing so Mu followers
   can commit the final real call; Mu normally commits an entry when it observes
   the following entry's first-undecided offset.

The final two items matter on CPU Soft-RoCE because its send queue can fill
during a million-operation burst. The original implementation ignored a failed
`ibv_post_send`, which could leave one replica waiting forever for a log entry.
Backpressure activates only when the provider rejects a post.

## Validated CloudLab Topology

The Mellanox ports on the two allocated machines were link-up but were not in a
shared L2 network. The direct CPU Ethernet network was therefore used, without
the FPGA path:

| Host | Ethernet interface | IPv4 address | Soft-RoCE device | GID index |
| --- | --- | --- | --- | --- |
| `pc160` | `enp135s0f0` | `192.168.40.30` | `rxe0` | `1` |
| `pc161` | `enp135s0f0` | `192.168.40.31` | `rxe0` | `1` |
| `pc162` | `enp135s0f0` | `192.168.40.32` | `rxe0` | `1` |
| `pc163` | `enp135s0f0` | `192.168.40.33` | `rxe0` | `1` |

On all four nodes, GID index `1` is the RoCE v2 IPv4-mapped GID. The active
Soft-RoCE MTU is 1024 bytes. The 3-replica runs use `pc160`--`pc162`; the
4-replica runs use all four hosts.

The matrix starts a fresh 64 MiB memcached registry on `pc160` for every
measurement. Because only four hosts were allocated, `pc160` also runs replica
1; there is no fifth dedicated registry host. Memcached is used for connection
metadata, not as a protocol replica. A deployment with a spare host can isolate
the registry without changing Hamband.

## Prerequisites

The tested hosts use Ubuntu 22.04, GCC 11, and Conan 1. Install the required
packages on both nodes:

```bash
sudo apt-get update
sudo apt-get install -y \
  build-essential cmake ninja-build python3-pip git unzip \
  rdma-core ibverbs-utils libibverbs-dev librdmacm-dev perftest \
  memcached libmemcached-dev libssl-dev pkg-config \
  libtbb-dev libcds-dev

python3 -m pip install --user 'conan<2'
export PATH="$HOME/.local/bin:$PATH"
```

The build profile is fixed at GCC 11 in `tools/gcc11-release.profile`. Use a
matching profile if the target cluster has a different compiler version.

## Create the Soft-RoCE Device

Run this on both CloudLab nodes from the RoCE repository:

```bash
cd "$HOME/Hamband/roce"
./tools/setup-host.sh
```

The script is idempotent. By default it creates `rxe0` over `enp135s0f0`.
Override those names when necessary:

```bash
ROCE_NETDEV=eth1 ROCE_DEVICE=rxe0 ./tools/setup-host.sh
```

Soft-RoCE configuration is not persistent across a reboot, so rerun the setup
script after either node restarts.

Verify the device and IPv4 GID:

```bash
rdma link show
ibv_devinfo -d rxe0
cat /sys/class/infiniband/rxe0/ports/1/gids/1
cat /sys/class/infiniband/rxe0/ports/1/gid_attrs/types/1
```

The GID type should be `RoCE v2`.

## Build

Build on every participating node:

```bash
cd "$HOME/Hamband/roce"
./tools/build.sh
```

The build script:

- uses the included GCC 11 Conan profile;
- rebuilds the local Dory/Hamband packages in dependency order;
- extracts the repository's pinned Junction and Turf archives when the old
  gitlinks are empty;
- builds `wellcoordination/build/bin/band` and `band-crdt`; and
- builds all WRDT/CRDT workload generators used by Figures 9--11.

Confirm the protocol binaries and at least the required generators:

```bash
test -x wellcoordination/build/bin/band
test -x wellcoordination/build/bin/band-crdt
test -x wellcoordination/build/bin/account-benchmark
test -x wellcoordination/build/bin/register-crdt-benchmark
test -x wellcoordination/build/bin/kvstore-benchmark
test -x wellcoordination/build/bin/smallbank-benchmark
```

## Optional Raw RoCE Check

Before running Hamband, a raw RC write test can verify the RoCE path. On
`pc160`:

```bash
ib_write_bw -d rxe0 -i 1 -x 1 -n 5000 --report_gbits
```

On `pc161`:

```bash
ib_write_bw -d rxe0 -i 1 -x 1 -n 5000 --report_gbits 192.168.40.30
```

The validated setup negotiated Ethernet, GID index 1, and MTU 1024. It reached
approximately 2.86 Gb/s with CPU Soft-RoCE.

## Generate the Account Workload

Run these commands on both nodes. Generating locally avoids any workload-file
transfer and produces the same deterministic files on each host.

```bash
ROOT="$HOME/Hamband/roce"
WORKLOAD="$ROOT/wellcoordination/workload"

mkdir -p "$WORKLOAD/2-1000000-100/account"
HAMBAND_WORKLOAD_DIR="$WORKLOAD" \
  "$ROOT/wellcoordination/build/bin/account-benchmark" 2 1000000 100
```

The resulting files are:

```text
wellcoordination/workload/2-1000000-100/account/1.txt
wellcoordination/workload/2-1000000-100/account/2.txt
```

Each replica receives 500,000 local operations. Node 1 receives conflicting
withdrawals and node 2 receives non-conflicting deposits.

## Run the Two-Node Experiment

The `band` command-line format used here is:

```text
band <id> <nodes> <operations> <write-percent> <usecase> <throughput> <failure>
```

For this validation:

- `nodes=2`
- `operations=1000000`
- `write-percent=100`
- `usecase=account`
- `throughput=1`
- `failure=0`

Start a fresh registry on `pc160`:

```bash
memcached -l 192.168.40.30 -p 9999
```

In another terminal, start node 1 on `pc160`:

```bash
ROOT="$HOME/Hamband/roce"
export DORY_RDMA_DEVICE=rxe0
export DORY_GID_INDEX=1
export DORY_REGISTRY_IP=192.168.40.30:9999
export HAMBAND_WORKLOAD_DIR="$ROOT/wellcoordination/workload"

"$ROOT/wellcoordination/build/bin/band" \
  1 2 1000000 100 account 1 0 | tee /tmp/hamband-roce-node1.log
```

Start node 2 on `pc161` at approximately the same time:

```bash
ROOT="$HOME/Hamband/roce"
export DORY_RDMA_DEVICE=rxe0
export DORY_GID_INDEX=1
export DORY_REGISTRY_IP=192.168.40.30:9999
export HAMBAND_WORKLOAD_DIR="$ROOT/wellcoordination/workload"

"$ROOT/wellcoordination/build/bin/band" \
  2 2 1000000 100 account 1 0 | tee /tmp/hamband-roce-node2.log
```

Do not reuse a registry containing keys from a previous run. Start a fresh
memcached process for each experiment.

## Validated Two-Node Result

The final successful run produced:

| Measurement | Node 1 | Node 2 |
| --- | ---: | ---: |
| Locally issued operations | 500,000 | 500,000 |
| Reported throughput (`ops/us`) | 0.0762054 | 0.0762037 |
| Reported throughput (`ops/s`) | 76,205 | 76,204 |
| Final balance | 4,752,077 | 4,752,077 |
| Finish barrier | `all nodes finished` | `all nodes finished` |

The workload was independently summed as:

```text
initial balance  =  100000
withdrawal sum   =  100000
deposit sum      = 4752077
expected balance = 4752077
```

Both logs were checked for failed work completions, exceptions, segmentation
faults, integrity drops, and non-permissible requests. No error markers were
present.

## Validated Paper Matrix

The completed CPU Soft-RoCE matrix is published at
[`results/hamband_roce_paper_4m.csv`](results/hamband_roce_paper_4m.csv). Its
scope is:

- 3 and 4 replicas;
- 4,000,000 total operations per configuration;
- Counter, Register, G-Set, PN-Set, 2P-Set, Account, Courseware, Project,
  Movie, Auction, YCSB, and SmallBank;
- 0%, 15%, 20%, and 25% writes for every workload; and
- additional 5% and 50% rows for YCSB and SmallBank.

That produces 104 unique rows: 52 for each replica count, 8 for each Figure 9
or Figure 10 workload, and 12 each for YCSB and SmallBank. Failure handling was
disabled (`0`) throughout. CRDT rows use one combined measurement; WRDT rows
use separate response and throughput executions. The CSV reports response time
as the arithmetic mean of the replica-local response values and throughput as
the minimum replica throughput.

The SSH orchestrator used `pc160` as both registry host and replica 1, then an
ordered prefix of `pc161`, `pc162`, and `pc163` for the remaining replicas. It
generated deterministic workloads independently on every active host, checked
that their hashes matched, required exactly 4,000,000 issued operations, and
accepted a row only after every replica reached the finish barrier with the
same state digest and no configured crash/integrity/error signature.

The CSV preserves exact per-row source revisions:

| Revision | Rows | Purpose |
| --- | ---: | --- |
| `1617b35` | 51 | Secondary Mu leader initialization stabilization |
| `0952264` | 53 | Later descendant with pipelined quorum completion tracking |

The revisions are intentionally visible in the `commit` column; this is not a
single-revision dataset. Both retain the same protocol decisions, while the
later revision prevents an already-completed pipelined quorum from being
missed. All four currently deployed CloudLab checkouts are at `0952264`.

The CSV was re-audited on August 6, 2026 for all 104 expected keys, exact
operation/status/transport fields, active per-node metric columns, and every
reported response mean and throughput minimum. The audit passed. Its SHA-256
is:

```text
65360c8bc608c4f37892a418b6ce3fa2d5b638f73d12808746c330ee2e018dca
```

## Hardware RoCE

For a hardware RoCE NIC, do not create `rxe0`. Configure the Ethernet/RoCE
network normally, identify the verbs device and routable RoCE v2 GID index,
then select them:

```bash
export DORY_RDMA_DEVICE=mlx5_0
export DORY_GID_INDEX=<roce-v2-gid-index>
```

The connection code will use GID/GRH addressing because the selected port's
link layer is Ethernet. Rebuild only when compiler or dependency settings
differ; changing the selected device or GID does not require source changes.

## Troubleshooting

### `Requested RDMA device ... was not found`

Run `ibv_devices` and set `DORY_RDMA_DEVICE` to an existing verbs device.

### `Failed to query RoCE GID`

Check the GID table under
`/sys/class/infiniband/<device>/ports/1/gids/` and select a populated index.
For this CloudLab setup, the correct value is `1`.

### Nodes wait during connection exchange

Confirm that both nodes use the same fresh memcached registry and can reach
`192.168.40.30:9999`. Also confirm that both selected GIDs are on the same
RoCE network.

### `rxe0` disappears

Soft-RoCE devices do not survive a reboot. Rerun `./tools/setup-host.sh`.

### Build uses the wrong Conan version

This repository uses Conan 1 recipes. Confirm `conan --version` reports 1.x
and ensure `$HOME/.local/bin` is in `PATH`.

### Memory allocation fails

The current Hamband configuration allocates a multi-gigabyte registered
buffer per process. Ensure each node has sufficient available memory and an
RDMA provider that can register the requested region.

## Current Scope

- Two-node account correctness and 3--4 replica paper workloads are validated
  over CPU Soft-RoCE.
- The published 4-replica matrix co-locates memcached and replica 1 on `pc160`;
  it does not use a separate fifth registry host.
- FPGA paths were intentionally excluded.
- Hardware-RoCE performance has not yet been measured.
- Replica counts above four have not been measured on RoCE.

These limits concern deployment and performance characterization, not the
validated protocol execution.

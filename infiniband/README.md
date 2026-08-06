# Hamband over InfiniBand

This directory contains the preserved original Hamband implementation for
InfiniBand clusters such as ACES.

It is a complete project snapshot from pre-RoCE commit `0de38f1`. The files in
this directory are not converted to RoCE and retain the behavior used by the
existing InfiniBand experiments.

## What Is Preserved

The InfiniBand copy retains:

- LID-based reliable-connection addressing;
- the original fixed `IBV_MTU_4096` queue-pair path MTU;
- the original RDMA-device selection behavior;
- Hamband and Mu protocol logic;
- leader, follower, quorum, and two-node support;
- conflict and dependency handling;
- failure-mode code and experiment scripts;
- benchmark generation and result formatting; and
- the ACES-oriented build and Slurm scripts.

No RoCE GID, GRH, Soft-RoCE, or queue-backpressure changes are present here.

## ACES Location

The established ACES deployment is:

```text
/scratch/user/u.js213354/Hamband
```

For this two-folder repository layout, a fresh deployment can instead use:

```text
/scratch/user/u.js213354/Hamband/infiniband
```

Some historical scripts contain the first path literally. Keep the established
ACES location or update only those deployment paths when using the nested
layout. Path changes do not require protocol changes.

## Requirements

The original project expects:

- Linux with an active InfiniBand verbs device;
- GCC and CMake/Ninja;
- Conan 1.x and the repository's Conan profiles;
- libibverbs and RDMA development headers;
- memcached and libmemcached;
- the original Dory/Hamband dependencies; and
- Slurm for the included cluster experiment scripts.

On ACES, load the same modules used by the existing experiments:

```bash
cd /scratch/user/u.js213354/Hamband/infiniband
source load_modules.sh
```

## Verify InfiniBand Before Building

Check that the selected nodes expose an active InfiniBand port:

```bash
ibv_devices
ibv_devinfo
```

The relevant port must report:

```text
state: PORT_ACTIVE
link_layer: InfiniBand
```

This preserved implementation intentionally rejects an Ethernet link layer.
Use the sibling `../roce/` project for Ethernet/RoCE ports.

## Build Hamband

The original build entry point is under `wellcoordination/`:

```bash
cd /scratch/user/u.js213354/Hamband/infiniband/wellcoordination
source build.sh
```

The enabled targets are controlled by
`wellcoordination/src/CMakeLists.txt`. Keep the same `band` and `band-main`
target selection used by the established ACES experiment.

After a successful build, confirm the executable:

```bash
test -x build/bin/band
```

## Account Workload

The account workload generator is:

```text
wellcoordination/benchmark/account-benchmark.cpp
```

Historical scripts place generated workloads under:

```text
wellcoordination/workload/<nodes>-<operations>-<write-percent>/account/
```

For example, a two-node, one-million-operation, 100%-write workload uses:

```text
wellcoordination/workload/2-1000000-100/account/1.txt
wellcoordination/workload/2-1000000-100/account/2.txt
```

Use the existing benchmark binary or compile the generator with the same
C++17 command used by the original project.

## Run with the Existing ACES Script

`runaccess.sh` is the established Slurm runner for the account experiment. It
starts a separate memcached registry, launches one Hamband process per replica,
waits for every worker, and stores logs under the workload directory.

Before submitting, verify these variables in the script:

```bash
DORY_HOME=/scratch/user/u.js213354/Hamband/infiniband
RESULT_LOC=/scratch/user/u.js213354/Hamband/infiniband/wellcoordination/workload
NUM_OPS=1000000
WRITE_PERC=100
MODE=band
USECASE=account
FAILURE=0
```

Then submit with Slurm:

```bash
sbatch runaccess.sh
```

The Slurm allocation needs one process node per replica and the registry node
expected by the script. Failure must remain `0` for a no-failure experiment.

## Run the Binary Directly

The account command format is:

```text
band <id> <nodes> <operations> <write-percent> <usecase> <throughput> <failure>
```

Example process arguments for a two-node throughput run are:

```bash
./wellcoordination/build/bin/band 1 2 1000000 100 account 1 0
./wellcoordination/build/bin/band 2 2 1000000 100 account 1 0
```

Before launching either process, set the same fresh memcached registry on both:

```bash
export DORY_REGISTRY_IP=<registry-ip>:9999
```

Launch both processes at approximately the same time and do not reuse registry
keys from a previous experiment.

## Validate a Run

A successful no-failure account run should show, on every replica:

- the expected number of locally issued operations;
- a final state;
- the same final balance on all replicas; and
- `all nodes finished`.

Scan every log for:

```bash
grep -Ein \
  'segmentation|core dumped|unexpected error|failed|not permissible|exception' \
  wellcoordination/workload/*/account/AE_results/*.log
```

For conflicting account calls, also verify that the integrity-drop and Dory
error counts are zero when the generated workload is expected to be valid.

## Two-Node Behavior

The preserved source includes the two-node quorum support added before the RoCE
work. With two replicas, Hamband runs with one leader and one follower. This is
independent of the RDMA transport and does not require RoCE changes.

## ACES Paper Matrix

`experiments/run_hamband_paper_aces.py` reproduces the Hamband workloads from
Figures 9--11 in one fixed ACES allocation. The full matrix uses:

- 3, 4, 5, 6, 7, and 8 replicas;
- 4,000,000 total operations per configuration;
- 0%, 15%, 20%, and 25% writes for every workload; and
- additional 5% and 50% rows for YCSB and SmallBank.

The allocation contains nine verified NUMA2 nodes: one registry and eight
ordered worker nodes. A run with fewer than eight replicas uses a prefix of the
same worker list, so the registry and worker-node mapping never change during
the matrix. Failure remains disabled (`0`) in every invocation.

Submit from the refreshed two-folder checkout:

```bash
cd /scratch/user/u.js213354/Hamband/infiniband
bash experiments/submit_hamband_paper_aces.sh
```

The submission helper reads the NUMA2 inventory maintained by the nonblocking
project, excludes every other CPU node, and asks Slurm to select nine nodes
atomically. Slurm keeps those resources for the complete job; there is no race
between an availability poll and reservation. To resume on an explicitly known
set, list the same nine nodes in allocation order:

```bash
HAMBAND_FIXED_NODES=ac002,ac015,ac026,ac029,ac030,ac037,ac043,ac044,ac046 \
  bash experiments/submit_hamband_paper_aces.sh
```

The job first runs 40K-operation account and counter smoke tests at 3, 4, and 8
replicas. It then checkpoints each validated full-matrix row to:

```text
results/hamband_infiniband_aces_4m.csv
```

For every execution, the runner requires exact issued-operation totals, a
finish barrier on every replica, identical final-state digests, and no crash,
malformed call, integrity drop, Dory error, or rejected WRDT request. The CSV
reports response time as the average of replica-local averages and throughput
as the minimum replica throughput, in both operations per microsecond and
operations per second. CRDT rows use one combined measurement run; WRDT rows
use separate response-time and throughput runs.

The Slurm request keeps the established four CPUs per Hamband process. It asks
for 16 GiB per node: the largest target path has two 4 GiB Mu buffers, one 3
GiB broadcast log, and less than 1 GiB of benchmark/application state, leaving
several GiB of headroom. This changes only the scheduler reservation and does
not alter protocol memory sizes or execution behavior.

## Important Separation Rule

Use only binaries from this `infiniband/` directory for an original InfiniBand
experiment. Do not start a peer built from `../roce/` in the same run because
the RoCE-capable copy exchanges additional transport metadata.

## Troubleshooting

### Port reports Ethernet

This copy requires InfiniBand. Select an InfiniBand verbs device or use the
RoCE project.

### Build refers to an old absolute path

The preserved scripts intentionally retain ACES-era paths. Update deployment
paths only; do not copy transport source from the RoCE directory.

### Nodes wait during connection exchange

Confirm all processes use the same fresh `DORY_REGISTRY_IP`, that memcached is
reachable, and that the InfiniBand fabric permits traffic between every pair of
replicas.

### Final states differ

Keep all processes alive through the `all nodes finished` barrier, check every
work-completion error, and confirm the workload files have matching headers and
expected operation counts.

## Related Project

The sibling [`../roce/`](../roce/) directory contains the independently
buildable RoCE-capable version and its validated CloudLab instructions.

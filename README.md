# Hamband

This repository contains two independent Hamband implementations in one
`master` branch. Choose the directory that matches the RDMA transport for the
experiment.

| Directory | Transport | Purpose |
| --- | --- | --- |
| [`infiniband/`](infiniband/) | InfiniBand | ACES implementation and validated 3--8 replica matrix |
| [`roce/`](roce/) | InfiniBand or RoCE | Validated 3--4 replica Soft-RoCE and hardware-RoCE matrices |

## Choosing a Version

For the original ACES InfiniBand experiments:

```bash
cd infiniband
```

For RoCE experiments:

```bash
cd roce
```

Each directory is a complete project with its own source tree, build files,
scripts, and README:

- [InfiniBand setup and experiments](infiniband/README.md)
- [RoCE setup and validated experiments](roce/README.md)

Do not mix binaries from the two directories in one run. All replicas in an
experiment must use binaries built from the same directory.

## Preservation Guarantee

The `infiniband/` directory began as the pre-RoCE `master` source from commit
`0de38f1`. It retains LID addressing, the fixed InfiniBand path MTU, and the
original transport behavior, with documented narrow replica-scaling and ACES
build fixes.

The `roce/` directory adds Ethernet GID/GRH addressing, active-MTU negotiation,
explicit device selection, and RoCE queue backpressure. Its README records the
exact revisions and topology used by the Soft-RoCE and hardware-RoCE result
rows. Hamband and Mu protocol decisions are unchanged.

The separation is intentional: future RoCE work can remain inside `roce/`
without changing the preserved InfiniBand implementation.

Published results are kept with their transport:

- [`infiniband/results/hamband_infiniband_aces_4m.csv`](infiniband/results/hamband_infiniband_aces_4m.csv)
- [`roce/results/hamband_soft_roce_paper_4m.csv`](roce/results/hamband_soft_roce_paper_4m.csv)
- [`roce/results/hamband_hardware_roce_paper_4m.csv`](roce/results/hamband_hardware_roce_paper_4m.csv)

# Hamband

This repository contains two independent Hamband implementations in one
`master` branch. Choose the directory that matches the RDMA transport for the
experiment.

| Directory | Transport | Purpose |
| --- | --- | --- |
| [`infiniband/`](infiniband/) | InfiniBand | Preserved original implementation |
| [`roce/`](roce/) | InfiniBand or RoCE | Validated RoCE-capable implementation |

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
- [RoCE setup and validated two-node experiment](roce/README.md)

Do not mix binaries from the two directories in one run. All replicas in an
experiment must use binaries built from the same directory.

## Preservation Guarantee

The `infiniband/` directory is the pre-RoCE `master` source from commit
`0de38f1`. It retains the original LID addressing, fixed InfiniBand path MTU,
device selection, scripts, and protocol implementation.

The `roce/` directory is the tested RoCE-capable source from commit `d9be264`.
Its transport layer adds Ethernet GID/GRH addressing, active-MTU negotiation,
explicit device selection, and Soft-RoCE queue backpressure. Hamband and Mu
protocol decisions are unchanged.

The separation is intentional: future RoCE work can remain inside `roce/`
without changing the preserved InfiniBand implementation.

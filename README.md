# Hamband: RDMA replicated data types

The source code of Hamband is presented in this repository. 

## Getting Started
This project needs several modules in order to run successfully. Load required modules with:
```sh
$ source load_modules.sh
```

## Compiling the source code

**Compiling benchmark of each use-case:** 
This project considers several use cases. For each use case, we prepared a benchmark code. To compile the code of each benchmark go to the wellcoordination/src/ directory and compile each benchmark with:
```sh
$ g++ --std=c++17 usecase-benchmark.cpp -o usecase-benchmark.out
```
**Compiling the source code of RDMA-based use-cases:**
In order to compile use-cases that will run with RDMA (Band and Mu), go to the wellcoordination directory and run:
```sh
$ source build.sh
```
**Compiling TCP Based CRDTS( Message passing):**
In order to compile TCP-based CRDTs implementation go to the tcpcrdt and run:
```sh
$ source compile.sh
```
## Running the project
For running the project on a cluster we prepared a script file. It should be noted that the cluster should support the RDMA connection between nodes and exploits Slurm cluster management. If the cluster does not exploit Slurm cluster management, you should run each command of the script file, step by step.

**Band/Mu:**

For example for Movie use-case in a without failure scenario:

Movie run:
	response/thorughput:
```sh
$ sbatch run.sh 4 4000000 100 {mu/band} 1 movie {0/1} 0
```
Movie results:
	response time result:
```sh
$ grep -r -w "total average" wellcoordination/workload/4-4000000-100/movie/AE_results/{mu/band}*response* | sort -t: -n -k2 | awk '{split($0,a); if(a[8] != "-nan") {sum += a[8]; count += 1;}} END{print sum/count;}'
```
	throughout results:
```sh
$ grep -r -w "throughput" wellcoordination/workload/4-4000000-100/movie/AE_results/{mu/band}*throughput* | sort -t: -n -k2 | awk '{split($0,a); max = 100; if(a[2] < max) max = a[2];} END{print max;}'
```

For example for Courseware use-case in a failure scenario:

Courseware failure run (response/thorughput):
```sh
$ sbatch run.sh 4 4000000 10 band 1 courseware {0/1} 0
$ sbatch run.sh 4 4000000 10 band-failure 1 courseware {0/1} 1
$ sbatch run.sh 4 4000000 10 band-failure 1 courseware {0/1} 2
```
Courseware failure results (response/thorughput):
```sh
$ grep -r -w "throughput" wellcoordination/workload/4-4000000-10/courseware/AE_results/*throughput*{no/follower/leader}* | sort -t: -n -k2 | awk '{split($0,a); max = 100; if(a[2] < max) max = a[2];} END{print max;}'
```

**TCP based CRDTs:**

For running the TCP-based CRDTs on a cluster we prepared a script file. You can run the script file with the following command with 3 input arguments:
```sh
$ sbatch run-tcp-crdt.sh arg1 arg2 arg3
```
arg1 is the number of operations.
arg2 is the number of times that you want to repeat experiments.
arg3 is the type of use-case that you want to run (counter, gset, register, orset and shop).

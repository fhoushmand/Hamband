#!/bin/bash
#SBATCH --nodes=4
#SBATCH --ntasks=4
#SBATCH --cpus-per-task=4
#SBATCH --output="result.log"
#SBATCH --mem=64G
#SBATCH --time=01:00:00     # HH:MM:SS or D-HH:MM:SS
#SBATCH --partition=cpu
nodes=($( scontrol show hostnames $SLURM_NODELIST ))
nnodes=${#nodes[@]}
last=$(( $nnodes - 1 ))
DORY_HOME="/scratch/user/u.js213354/Hamband/"
RESULT_LOC="/scratch/user/u.js213354/Hamband/wellcoordination/workload/"
RESULTS_DIR_NAME="AE_results"
NUM_OPS=1000000
WRITE_PERC="10"
MODE="band" # mu, band, band-crdt, band-crdt-failure band-failure
REP=1 # number of reps
USECASE="account" # name of the usecase: project, courseware, movie, gset, counter, account
THROUGHPUT=0 # 1 to calculate throughput, 0 to calculate response times
FAILURE=0 # 0 for no failure, 1 for follower and 2 for leader failure
MEMCACHED_BIN="/scratch/user/u.js213354/memcached/bin/memcached"
EXECUTION="response"
if [ "$THROUGHPUT" -eq 1 ]; then
    EXECUTION="throughput"
fi
CRASH="no"
if [ "$FAILURE" -eq 1 ]; then
    CRASH="leader"
fi
if [ "$FAILURE" -eq 2 ]; then
    CRASH="follower"
fi
for n in $( seq 3 3 ); do
        for p in $WRITE_PERC; do
                BENCH_DIRECTORY=$RESULT_LOC$n-$NUM_OPS-$p/$USECASE;
                echo $BENCH_DIRECTORY;
                if [ ! -d "$BENCH_DIRECTORY" ]; then
                        mkdir -p $BENCH_DIRECTORY;
                        mkdir -p $BENCH_DIRECTORY/$RESULTS_DIR_NAME;
                        /scratch/user/u.js213354/Hamband/wellcoordination/benchmark/$USECASE-benchmark.out $n $NUM_OPS $p
                        echo "benchmark generated";
                fi
        done
done
hostlist=""
for i in $( seq 0 $last ); do
        hostlist+="${nodes[$i]} "
done
echo $hostlist
mkdir -p $BENCH_DIRECTORY/$RESULTS_DIR_NAME;
ip0=$(ping -c 1 ${nodes[0]} | grep 'PING' | awk -F'[()]' '{print $2}')
r=1
for n in $( seq 3 3 ); do
        for p in $WRITE_PERC; do
                ssh ${nodes[0]} "$MEMCACHED_BIN -vv -p 9999 > $BENCH_DIRECTORY/$RESULTS_DIR_NAME/memcached-$r.log 2>&1"&
                memcached_pid=$!
                sleep 2;
                worker_pids=()
                for i in $( seq 1 $n ); do
                        printf "%s\n" "ssh ${nodes[$i]} 'cd ${DORY_HOME}; export DORY_REGISTRY_IP=${ip0}:9999; ./wellcoordination/build/bin/$MODE $i $n $NUM_OPS $p $USECASE $THROUGHPUT $FAILURE > $RESULT_LOC$n-$NUM_OPS-$p/$USECASE/$RESULTS_DIR_NAME/$MODE-$i-$r-$EXECUTION-$CRASH.log 2>&1'"
                        ssh ${nodes[$i]} "cd ${DORY_HOME}; export DORY_REGISTRY_IP=${ip0}:9999; ./wellcoordination/build/bin/$MODE $i $n $NUM_OPS $p $USECASE $THROUGHPUT $FAILURE > $RESULT_LOC$n-$NUM_OPS-$p/$USECASE/$RESULTS_DIR_NAME/$MODE-$i-$r-$EXECUTION-$CRASH.log 2>&1"&
                        worker_pids+=( $! )
                done
                worker_status=0
                for worker_pid in "${worker_pids[@]}"; do
                        wait "$worker_pid" || worker_status=1
                done
                ssh ${nodes[0]} "bash -s" <./kill-memcached.sh
                wait "$memcached_pid" 2>/dev/null || true
                if [ "$worker_status" -ne 0 ]; then
                        exit "$worker_status"
                fi
                sleep 2;
        done
        ssh ${nodes[0]} "bash -s" <./kill-memcached.sh
done

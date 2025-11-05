#!/bin/bash
#SBATCH --nodes=4
#SBATCH --ntasks=4
#SBATCH --cpus-per-task=4
#SBATCH --output="result.log"
#SBATCH --mem=64G
#SBATCH --time=00:15:00     # HH:MM:SS or D-HH:MM:SS
#SBATCH --partition=cpu
nodes=($( scontrol show hostnames $SLURM_NODELIST ))
nnodes=${#nodes[@]}
last=$(( $nnodes - 1 ))
DORY_HOME="/scratch/user/u.js213354/Hamband/"
RESULT_LOC="/scratch/user/u.js213354/Hamband/wellcoordination/workload/"
RESULTS_DIR_NAME="AE_results"
NUM_OPS=400000
WRITE_PERC="10"
MODE="band" # mu, band, band-crdt, band-crdt-failure band-failure
REP=1 # number of reps
USECASE="account" # name of the usecase: project, courseware, movie, gset, counter
THROUGHPUT=0 # 1 to calculate throughput, 0 to calculate response times
FAILURE=1 # 0 for no failure, 1 for follower and 2 for leader failure
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
                ssh ${nodes[0]} 'memcached -vv -p 9999'&
                sleep 2;
                for i in $( seq 1 $n ); do
                        printf "ssh ${nodes[$i]} 'cd ${DORY_HOME}; export DORY_REGISTRY_IP=${ip0}:9999; ./wellcoordination/build/bin/$MODE $i $n $NUM_OPS $p $USECASE $THROUGHPUT $FAILURE > $RESULT_LOC$n-$NUM_OPS-$p/$USECASE/$RESULTS_DIR_NAME/$MODE-$i-$r-$EXECUTION-$CRASH.log'\n"
                        ssh ${nodes[$i]} "cd ${DORY_HOME}; export DORY_REGISTRY_IP=${ip0}:9999; ./wellcoordination/build/bin/$MODE $i $n $NUM_OPS $p $USECASE $THROUGHPUT $FAILURE > $RESULT_LOC$n-$NUM_OPS-$p/$USECASE/$RESULTS_DIR_NAME/$MODE-$i-$r-$EXECUTION-$CRASH.log"&
                done
                sleep 100;
                ssh ${nodes[0]} "bash -s" <./kill-memcached.sh
                sleep 2;
        done
        ssh ${nodes[0]} "bash -s" <./kill-memcached.sh
done
sleep 300

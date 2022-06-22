#!/bin/bash -l

#SBATCH --nodes=8
#SBATCH --ntasks=8
#SBATCH --cpus-per-task=8
#SBATCH --output="result.log"
#SBATCH --mem=30G
#SBATCH -p batch # This is the default partition, you can use any of the following; intel, batch, highmem, gpu
#SBATCH --constraint="amd"
##SBATCH --nodelist="c[03],c[08],c[12-13],c[17], c[19]"
##SBATCH --exclusive

nodes=($( scontrol show hostnames $SLURM_NODELIST ))
nnodes=${#nodes[@]}
last=$(( $nnodes - 1 ))

DORY_HOME="/rhome/fhous001/farzin/FastChain/dory"
RESULT_LOC="/rhome/fhous001/farzin/FastChain/dory/wellcoordination/workload/"
RESULTS_DIR_NAME="AE_results"

NUM_NODES=$1
NUM_OPS=$2
WRITE_PERC=$3
MODE=$4 # mu, band, band-crdt, band-crdt-failure band-failure
REP=$5 # number of reps
USECASE=$6 # name of the usecase: project, courseware, movie, gset, counter
FAILURE=$7 # 0 for no failure, 1 for follower and 2 for leader failure

CRASH="normal"
if [ "$FAILURE" -eq 1 ]; then
    CRASH="failure"
fi


if [ "$#" -ne 6 ]; then
    echo "Illegal number of parameters"
fi
BENCH_DIRECTORY=$RESULT_LOC$NUM_NODES-$NUM_OPS-$WRITE_PERC/$USECASE;
echo $BENCH_DIRECTORY;
if [ ! -d "$BENCH_DIRECTORY" ]; then
        mkdir -p $BENCH_DIRECTORY;
        /rhome/fhous001/farzin/FastChain/dory/wellcoordination/benchmark/$USECASE-benchmark.out $NUM_NODES $NUM_OPS $WRITE_PERC;
        echo "benchmark generated";

fi

hostlist=""
for i in $( seq 0 $last ); do
        hostlist+="${nodes[$i]} "
done
echo $hostlist

mkdir -p $BENCH_DIRECTORY/$RESULTS_DIR_NAME;

#for n in $( seq 3 6 ); do
        for r in $( seq $REP $REP ); do
                ssh ${nodes[0]}.ib.hpcc.ucr.edu 'memcached -vv -p 9999'&
                sleep 2;
                
                for i in $( seq 1 $NUM_NODES ); do
                        printf "ssh ${nodes[$i]}.ib.hpcc.ucr.edu 'cd ${DORY_HOME}; export DORY_REGISTRY_IP=${nodes[0]}:9999; ./wellcoordination/build/bin/$MODE $i $NUM_NODES $NUM_OPS $WRITE_PERC $USECASE $THROUGHPUT $FAILURE > $RESULT_LOC$NUM_NODES-$NUM_OPS-$WRITE_PERC/$USECASE/$RESULTS_DIR_NAME/$MODE-$i-$r-$CRASH.log'\n"&
                        ssh ${nodes[$i]}.ib.hpcc.ucr.edu "cd ${DORY_HOME}; export DORY_REGISTRY_IP=${nodes[0]}:9999; ./wellcoordination/build/bin/$MODE $i $NUM_NODES $NUM_OPS $WRITE_PERC $USECASE $THROUGHPUT $FAILURE > $RESULT_LOC$NUM_NODES-$NUM_OPS-$WRITE_PERC/$USECASE/$RESULTS_DIR_NAME/$MODE-$i-$r-$CRASH.log"&
                done
                sleep 100;
		printf "here\n";
                ssh ${nodes[0]}.ib.hpcc.ucr.edu "bash -s" <./kill-memcached.sh
                sleep 2;
        done
        ssh ${nodes[0]}.ib.hpcc.ucr.edu "bash -s" <./kill-memcached.sh
# done

sleep 300

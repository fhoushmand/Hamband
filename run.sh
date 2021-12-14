#!/bin/bash

#SBATCH --nodes=5
#SBATCH --ntasks=5
#SBATCH --cpus-per-task=8
#SBATCH --output="result.log"
#SBATCH --mem=30G
#SBATCH -p short # This is the default partition, you can use any of the following; intel, batch, highmem, gpu
#SBATCH --exclusive
#SBATCH --constraint="amd"

nodes=($( scontrol show hostnames $SLURM_NODELIST ))
nnodes=${#nodes[@]}
last=$(( $nnodes - 1 ))

DORY_HOME="/rhome/fhous001/farzin/FastChain/dory"
RESULT_LOC="/rhome/fhous001/farzin/FastChain/dory/wellcoordination/workload/"

NUM_NODES=$1
NUM_OPS=$2
WRITE_PERC=$3
MODE=$4 # mu or hamsaz
REP=$5 # number of reps
USECASE=$6 # name of the usecase: account, courseware, gset, counter
THROUGHPUT=$7 # 1 to calculate throughput, 0 to calculate response times


if [ "$#" -ne 6 ]; then
    echo "Illegal number of parameters"
fi
BENCH_DIRECTORY=$RESULT_LOC$NUM_NODES-$NUM_OPS-$WRITE_PERC/$USECASE;
echo $BENCH_DIRECTORY;
if [ ! -d "$BENCH_DIRECTORY" ]; then
        mkdir -p $BENCH_DIRECTORY;
        /rhome/fhous001/farzin/FastChain/dory/wellcoordination/src/$USECASE-benchmark.out $NUM_NODES $NUM_OPS $WRITE_PERC;
        echo "benchmark generated";

fi

hostlist=""
for i in $( seq 0 $last ); do
        hostlist+="${nodes[$i]} "
done
echo $hostlist

mkdir -p $BENCH_DIRECTORY/results;

# MODE=mu
#for n in $( seq 3 6 ); do
        for s in $( seq 1 10 ); do
                r=$REP
                #printf "ssh ${nodes[0]}.ib.hpcc.ucr.edu 'cd ${DORY_HOME}; memcached -vv -p 9999'"
                ssh ${nodes[0]}.ib.hpcc.ucr.edu 'memcached -vv -p 9999'&
                sleep 2;
              
                for i in $( seq 1 $NUM_NODES ); do
                        #printf "ssh ${nodes[$i]}.ib.hpcc.ucr.edu 'cd ${DORY_HOME}; export DORY_REGISTRY_IP=${nodes[0]}:9999; numactl --membind 0 ./crash-consensus/demo/using_conan_fully/build/bin/main-st $i 4096 1 > $i.log'"
                        #ssh ${nodes[$i]}.ib.hpcc.ucr.edu "cd ${DORY_HOME}; export DORY_REGISTRY_IP=${nodes[0]}:9999; numactl --membind 0 ./wellcoordination/build/bin/hamsaz 2 $i 10000 10 > $i.log&"
                        printf "ssh ${nodes[$i]}.ib.hpcc.ucr.edu 'cd ${DORY_HOME}; export DORY_REGISTRY_IP=${nodes[0]}:9999; ./wellcoordination/build/bin/$MODE $i $NUM_NODES $NUM_OPS $WRITE_PERC $USECASE > $RESULT_LOC$NUM_NODES-$NUM_OPS-$WRITE_PERC/$USECASE/results/$MODE-$i-$r$s.log'\n";
                        ssh ${nodes[$i]}.ib.hpcc.ucr.edu "cd ${DORY_HOME}; export DORY_REGISTRY_IP=${nodes[0]}:9999; ./wellcoordination/build/bin/$MODE $i $NUM_NODES $NUM_OPS $WRITE_PERC $USECASE $THROUGHPUT > $RESULT_LOC$NUM_NODES-$NUM_OPS-$WRITE_PERC/$USECASE/results/$MODE-$i-$r$s.log&";
                done
                #numactl --membind 0 
                sleep 30;
                ssh ${nodes[0]}.ib.hpcc.ucr.edu "bash -s" <./kill-memcached.sh
                sleep 2;
        done
        ssh ${nodes[0]}.ib.hpcc.ucr.edu "bash -s" <./kill-memcached.sh
        sleep 2;
        # MODE=hamsaz
        # for r in $( seq $REP $REP ); do
        #         #printf "ssh ${nodes[0]}.ib.hpcc.ucr.edu 'cd ${DORY_HOME}; memcached -vv -p 9999'"
        #         ssh ${nodes[0]}.ib.hpcc.ucr.edu 'memcached -vv -p 9999'&
        #         sleep 2;
        #         for i in $( seq 1 $NUM_NODES ); do
        #                 #printf "ssh ${nodes[$i]}.ib.hpcc.ucr.edu 'cd ${DORY_HOME}; export DORY_REGISTRY_IP=${nodes[0]}:9999; numactl --membind 0 ./crash-consensus/demo/using_conan_fully/build/bin/main-st $i 4096 1 > $i.log'"
        #                 #ssh ${nodes[$i]}.ib.hpcc.ucr.edu "cd ${DORY_HOME}; export DORY_REGISTRY_IP=${nodes[0]}:9999; numactl --membind 0 ./wellcoordination/build/bin/hamsaz 2 $i 10000 10 > $i.log&"
        #                 printf "ssh ${nodes[$i]}.ib.hpcc.ucr.edu 'cd ${DORY_HOME}; export DORY_REGISTRY_IP=${nodes[0]}:9999; numactl --membind 0 ./wellcoordination/build/bin/$MODE $i $NUM_NODES $NUM_OPS $WRITE_PERC $USECASE > $RESULT_LOC$NUM_NODES-$NUM_OPS-$WRITE_PERC/$USECASE/results/$MODE-$i-$r.log'\n";
        #                 ssh ${nodes[$i]}.ib.hpcc.ucr.edu "cd ${DORY_HOME}; export DORY_REGISTRY_IP=${nodes[0]}:9999; numactl --membind 0 ./wellcoordination/build/bin/$MODE $i $NUM_NODES $NUM_OPS $WRITE_PERC $USECASE > $RESULT_LOC$NUM_NODES-$NUM_OPS-$WRITE_PERC/$USECASE/results/$MODE-$i-$r.log&";
        #         done
        #         sleep 100;
        #         ssh ${nodes[0]}.ib.hpcc.ucr.edu "bash -s" <./kill-memcached.sh
        #         sleep 2;
        # done



# done

# sleep 100

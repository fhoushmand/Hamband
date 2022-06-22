#!/bin/bash -l

#SBATCH --nodes=7
#SBATCH --ntasks=7
#SBATCH --cpus-per-task=7
#SBATCH --output="result.log"
#SBATCH --mem=50G
##SBATCH --nodelist="c[03],c[08],c[12-13],c[17], c[19]"
#SBATCH -p short # This is the default partition, you can use any of the following; intel, batch, highmem, gpu
#SBATCH --constraint="amd"

nodes=($( scontrol show hostnames $SLURM_NODELIST ))
nnodes=${#nodes[@]}
last=$(( $nnodes - 1 ))

DORY_HOME="/rhome/fhous001/farzin/FastChain/dory/"
RESULT_LOC="/rhome/fhous001/farzin/FastChain/dory/wellcoordination/workload/"

NUM_OPS=$1
m=$2 # number of nodes
REP=$3 # number of reps
PERCENTAGES=$4
USECASE=$5 # name of the usecase


if [ "$#" -ne 5 ]; then
    echo "Illegal number of parameters"
fi

hostlist=""
for i in $( seq 0 $last ); do
        hostlist+="${nodes[$i]} "
done
echo $hostlist

#PERCENTAGES="5 15 25";

MODE=tcp
#for n in $( seq 2 2 ); do
        n=$((m-1))
        for p in $PERCENTAGES; do
                BENCH_DIRECTORY=$RESULT_LOC$m-$NUM_OPS-$p/$USECASE;
                if [ ! -d "$BENCH_DIRECTORY" ]; then
                        mkdir -p $BENCH_DIRECTORY;
                        /rhome/fhous001/farzin/FastChain/dory/wellcoordination/benchmark/$USECASE-benchmark.out $m $NUM_OPS $p;
                        echo "benchmark generated";
                fi
                mkdir -p $BENCH_DIRECTORY/results;
                for r in $( seq $REP $REP ); do
                
                        for i in $( seq 0 $n ); do
                                j=$(($i+1))
                                #sleep 10
                                #printf "ssh ${nodes[$i]}.ib.hpcc.ucr.edu 'cd ${DORY_HOME}; export DORY_REGISTRY_IP=${nodes[0]}:9999; numactl --membind 0 ./crash-consensus/demo/using_conan_fully/build/bin/main-st $i 4096 1 > $i.log'"
                                #ssh ${nodes[$i]}.ib.hpcc.ucr.edu "cd ${DORY_HOME}; export DORY_REGISTRY_IP=${nodes[0]}:9999; numactl --membind 0 ./wellcoordination/build/bin/band 2 $i 10000 10 > $i.log&"
                                printf "ssh ${nodes[$i]}.ib.hpcc.ucr.edu 'cd ${DORY_HOME}; ./tcpcrdt/build/tcp-main-crdt $j $m $NUM_OPS $p $USECASE $hostlist > $RESULT_LOC$m-$NUM_OPS-$p/$USECASE/results/$MODE-$j-$r.log'\n";
                                ssh ${nodes[$i]}.ib.hpcc.ucr.edu "module load valgrind; cd ${DORY_HOME}; ./tcpcrdt/build/tcp-main-crdt $j $m $NUM_OPS $p $USECASE $hostlist > $RESULT_LOC$m-$NUM_OPS-$p/$USECASE/results/$MODE-$j-$r.log;"&
				sleep 1;
                        done
                        sleep 200;
                done
                sleep 2;
        done
        sleep 2;        
#done

sleep 500000

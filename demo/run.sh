#!/bin/bash

#SBATCH --nodes=4
#SBATCH --ntasks=4
#SBATCH --cpus-per-task=4
#SBATCH --output="result.log"
#SBATCH --mem=5G
#SBATCH --nodelist i05,i06,i07,i08
#SBATCH -p short # This is the default partition, you can use any of the following; intel, batch, highmem, gpue

nodes=($( scontrol show hostnames $SLURM_NODELIST ))
nnodes=${#nodes[@]}
last=$(( $nnodes - 1 ))

DORY_HOME="/rhome/fhous001/farzin/FastChain/dory/demo"

hostlist=""
for i in $( seq 0 $last ); do
        hostlist+="${nodes[$i]} "
done
echo $hostlist



#printf "ssh ${nodes[0]}.ib.hpcc.ucr.edu 'cd ${DORY_HOME}; memcached -vv -p 9999'"
ssh ${nodes[0]}.ib.hpcc.ucr.edu 'cd ${DORY_HOME}; memcached -vv -p 9999'&
sleep 5;
for i in $( seq 1 $last ); do
        #printf "ssh ${nodes[$i]}.ib.hpcc.ucr.edu 'cd ${DORY_HOME}; export DORY_REGISTRY_IP=${nodes[0]}:9999; numactl --membind 0 ./crash-consensus/demo/using_conan_fully/build/bin/main-st $i 4096 1 > $i.log'"
        ssh ${nodes[$i]}.ib.hpcc.ucr.edu "cd ${DORY_HOME}; export DORY_REGISTRY_IP=${nodes[0]}:9999; numactl --membind 0 ./build/bin/dorydemo $i > $i.log &"
done

sleep 240

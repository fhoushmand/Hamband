#!/bin/bash

PERCENTAGES="5 15 25";
USECASE="counter orset register gset shop"
for n in $( seq 3 7 ); do
    for u in $USECASE; do    
        for p in $PERCENTAGES; do
            sbatch run-tcp-crdt.sh 4000000 $n 1 $p $u
            sleep 200
            fcancel
            sleep300
        done  
    done    
done

sleep 500000
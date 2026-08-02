#!/bin/sh

sbatch run.sh 4 4000000 5 band-crdt 1 counter 1 1;
sleep 100;
fcancel;
sbatch run.sh 4 4000000 5 band-crdt 1 counter 0 1;
sleep 100;
fcancel;


sbatch run.sh 4 4000000 5 band-crdt 1 orset 1 1;
sleep 100;
fcancel;
sbatch run.sh 4 4000000 5 band-crdt 1 orset 0 1;
sleep 100;
fcancel;

!/bin/bash

#USECASE="counter gset register"
USECASE="orset"
#USECASE="orset shop"
PERCENTAGES="15 25";
#PERCENTAGES="15 25";
fcancel
for u in $USECASE; do
        for p in $PERCENTAGES; do
                for n in $( seq 3 7 ); do
                        sbatch run-tcp-crdt.sh 100000 $n 1 $p $u &
                        echo "check $n $p $u"
                        sleep 100
                        fcancel
                        #sleep 60
                done
        done
done

sleep 300

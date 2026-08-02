!/bin/bash

#USECASE="counter gset register"
#USECASE="orset shop"
#PERCENTAGES="5 10 15";
USECASE="gset"
#PERCENTAGES="25";
PERCENTAGES="5 15 25";
fcancel
for n in $( seq 3 7 ); do
        for p in $PERCENTAGES; do
                for u in $USECASE; do
                        sbatch runcrdt.sh $n 4000000 $p band-crdt 1 $u 0 &
                        echo "check $n $p $u"
                        sleep 200
                        fcancel
                        sleep 60
                done
        done
done

sleep 300

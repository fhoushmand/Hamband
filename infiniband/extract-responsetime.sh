#!/bin/bash

PERCENTAGES="25 15 5";

for p in $PERCENTAGES; do
	printf $p" :\n";
       for n in $( seq 3 7 ); do
    #    printf $n" :\n";
		grep -r -w "sum_response_time:" wellcoordination/workload/$n-4000000-$p/$1/results/* | sort -t: -n -k2 | head -1 | awk '{split($0,a); print 4000000000/a[2]}';

	done
done

#grep -r -w "total average" wellcoordination/workload/4-4000000-50/courseware/results/hamsaz* | sort -t: -n -k2 | awk '{split($0,a); sum += a[8]; count += 1;} END{print sum/count}'
# for p in $PERCENTAGES; do
# 	printf "prec: "$p" "; 
#         for n in $( seq 3 7 ); do
# 		printf "node: "$n" \n";
# 		for i in $( seq 0 4 ) ; do
# 			grep -r -w "calls to $i" wellcoordination/workload/4-4000000-$p/$1/results/$2* | sort -t: -n -k2 | awk '{split($0,a); if(a[9] != "-nan") {sum += a[9]; count += 1;}} END{print sum/count;}'
# 		done
# 		printf "###\n";
# 	done
# 	printf "****\n";
# done

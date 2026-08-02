grep -r throughput: wellcoordination/workload/6-1000000-90/courseware/results/
| sort -t: -n -k2


grep -r throughput: wellcoordination/workload/4-1000000-10/courseware/results/*-1.log | sort -t: -n -k2 | awk '{split($0,a); print a[2];}'

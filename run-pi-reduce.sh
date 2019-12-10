
#!/bin/bash

printf "[Step 1 of 2] Compiling MPI Code...\n"
mpicc pi-reduce.c -o pi-reduce

N=10
printf "[Step 2 of 2] Running MPI code on $N nodes...\n\n"
mpirun -n $N ./pi-reduce
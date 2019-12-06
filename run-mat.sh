
#!/bin/bash

printf "[Step 1 of 2] Compiling MPI Code (PThread Enabled)...\n"
mpicc matrix.c -o matrix -pthread

N=12
printf "[Step 2 of 2] Running MPI code on $N nodes...\n\n"
mpirun -n $N ./matrix
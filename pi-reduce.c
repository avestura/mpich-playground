/**
 * Title: Distributed calculating PI using MPICH2 Reduce
 * This file is licensed under GNU General Public License
 * By: Aryan Ebrahimpour (https://avestura.dev)
 * GitHub: https://github.com/avestura
 * At: Iran University of Science and Technology, Tehran, Iran
**/

#include <mpi/mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

double in_range_rnd(double min, double max, int rank){
    srand(time(0) + rank);
    return (double)rand()/(double)RAND_MAX * (max - min) + min;
}

bool is_in_circle(double x, double y){
    return (x*x) + (y*y) < 1.0;
}

long long* calculate_partial_pi(int rank){

    long long N = in_range_rnd(10000, 100000, rank);
    printf("[Rank = %d]: chose N = %lld.\n", rank, (long long)N);
    
    long long n = 0;
    for(int i = 0; i < N; i++){
        double x = in_range_rnd(0, 1.0, rank + i);
        double y = in_range_rnd(0, 1.0, (rank + i + 1.0));
        if(is_in_circle(x, y)){
            n++;
        }
    }

    long long* answer = malloc(sizeof(long long) * 2);
    answer[0] = N;
    answer[1] = n;
    return answer;
}

int main(int argc, char** argv){

    long long piData[2];

    MPI_Init(NULL, NULL);
    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    long long* calc = calculate_partial_pi(world_rank);

    MPI_Reduce(calc, &piData, 2, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    long long N = piData[0];
    long long n = piData[1];

    if(world_rank == 0){
        printf("FINAL REDUCED: N = %lld, n = %lld\n", N, n);
        printf("PI = %f\n", 4.0 * ((double)n/(double)N));
    }

    
    MPI_Finalize();
}

/**
 * Title: Distributed calculating PI using MPICH2 and Ring Topology
 * This file is licensed under GNU General Public License
 * By: Aryan Ebrahimpour (https://aryan.software)
 * GitHub: https://github.com/0xaryan
 * At: Iran University of Science and Technology, Tehran, Iran
**/

#include <mpi/mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

bool is_master(int rank) { return rank == 0; }
bool is_last_rank(int rank, int world_size) { return rank == world_size - 1; }

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
    MPI_Init(NULL, NULL);
    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if(world_size < 2){
        fprintf(stderr, "LOL! World size can't be less than 2\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    long long* calc = calculate_partial_pi(world_rank);

    if(is_last_rank(world_rank, world_size)){
        MPI_Send(calc, 2, MPI_LONG_LONG, world_rank - 1, 0, MPI_COMM_WORLD);
    }
    else if(is_master(world_rank)){
        long long buffer[2];
        MPI_Recv(&buffer, 2, MPI_LONG_LONG, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        long long myN = calc[0]; long long myn = calc[1];
        long long prevN = buffer[0]; long long prevn = buffer[1];
        long long N = prevN + myN; long long n = prevn + myn;
        printf("Final Result: N = %lld, n = %lld\n", N, n);
        printf("PI = %f\n", 4.0 * ((double)n/(double)N));
    }
    else {
        long long buffer[2];
        MPI_Recv(&buffer, 2, MPI_LONG_LONG, world_rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        long long myN = calc[0];
        long long myn = calc[1];
        long long prevN = buffer[0];
        long long prevn = buffer[1];
        long long N = prevN + myN;
        long long n = prevn + myn;
        long long sendBuffer[2] = {N, n};
        MPI_Send(&sendBuffer, 2, MPI_LONG_LONG, world_rank - 1, 0, MPI_COMM_WORLD);
    }
    
    MPI_Finalize();
}
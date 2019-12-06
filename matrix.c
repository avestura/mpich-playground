/**
 * Title: Distributed calculating product of two large matrix using MPICH2
 * This file is licensed under GNU General Public License
 * By: Aryan Ebrahimpour (https://aryan.software)
 * GitHub: https://github.com/0xaryan
 * At: Iran University of Science and Technology, Tehran, Iran
**/

#include <mpi/mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

#define N 64

long long product[N][N];
long rec_count = 0;

void init_p(){
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            product[i][j] = -1;
        }
    }
}

// Calculates how many nodes a cell should calculate
int number_to_solve(int nodes, bool last_node) {
    int base = ((N * N) / nodes);
    int rem = ((N * N) % nodes);
    return base + (last_node ? rem : 0);
}

// Calculates where should a node start calculating from
int start_index(int rank, int nodes, bool last_node) { return rank * number_to_solve(nodes, false); }

// Calculates where a node should end calculating
int end_index(int rank, int nodes, bool last_node) {
    return start_index(rank, nodes, false) + number_to_solve(nodes, last_node) - 1;
}

// Transforms an end or start index to its equvalient matrix index.
int* index2location(int n) {
    int* res = malloc(sizeof(int) * 2);
    res[0] = n / N;
    res[1] = n % N;
    return res;
}

bool is_master(int world_rank) { return world_rank == 0; }

// Prints calculation info
void print_info(int world_size) {
    int to_solve = number_to_solve(world_size, false);
    int to_solve_last = number_to_solve(world_size, true);
    printf("Initializing world with size %d to solve [%d*%d]*[%d*%d] product matrix.\n", world_size, N, N, N, N);
    printf("Every node will solve %d cells of product matrix", to_solve);
    if(to_solve != to_solve_last){
        printf(", except last items which solves %d items.", to_solve_last);
    }
    printf("\n");
}

// Check if world size is valid
void check_world_size(int world_size){
    if(world_size < 2){
        fprintf(stderr, "lol what! World size can't be less than 2\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
}

// Gets value A[i][j]
long long get_A(int i, int j) { return i * j; }

// Gets value B[i][_]
long long get_B(int i) { return i * i; }

// Calculates result of a cell in product matrix
long long calc_cell(int i, int j) {
    long long result = 0;
    for(int k = 0; k < N; k++){
        result += get_A(i, k) + get_B(k);
    }
    return result;
}

// Prints products matrix (Master only)
void print_mat(){
    for(int i = 0; i < N; i++){
        printf("%d: [", i);
        for(int j = 0; j < N; j++){
            printf("%lld, ", product[i][j]);
        }
        printf("]\n\n");
    }
}

// Checks if product fully calculated (Master only)
void check_if_product_calculated(){
    if(rec_count == N * N){
        printf("Done calculating product matrix.\n\n");
        print_mat();
        pthread_exit(NULL);
        exit(0);
    }
}

// Update product matrix (Master only)
void update_pmat(int i, int j, long long val){
    if(product[i][j] == -1){
        product[i][j] = val;
        rec_count++;
        check_if_product_calculated();
    }
}

// Tell master that a slave calculated a slot (Slave only)
void tell_master(int rank, int i, int j, long long val){
    long long data[4] = { rank, i, j, val };
    MPI_Request req;
    MPI_Send(&data, 4, MPI_LONG_LONG, 0, 0, MPI_COMM_WORLD);
}

// Calculate a range of slots of product matrix and update the product matrix
void calculate_area(int rank, int nodes, int last_node, bool is_master){
    int* start = index2location(start_index(rank, nodes, last_node));
    int* end   = index2location(end_index(rank, nodes, last_node));
    int s_i = start[0]; int s_j = start[1];
    int e_i = end[0];   int e_j = end[1];

    printf("Node %d will solve P[%d][%d] to P[%d][%d].\n", rank, s_i, s_j, e_i, e_j);

    int i, j;
    bool first_flag = false;
    for(i = s_i; i <= e_i; i++){
        for(j = 0; j <= N; j++){
            if(i == s_i && first_flag) {
                j = s_j; first_flag = true;
            }
            if(i == e_i && j > e_j) break;

            long long cell = calc_cell(i, j);
            if(is_master) {
                update_pmat(i, j, cell);
            }
            else {
                 tell_master(rank, i, j, cell);
            }
        }
    }
}

// Master's recieve thread (Didn't use Irecv due to limitations)
void* recieve_thread(){
    while(true){
        long long data[4];   
        MPI_Recv(&data, 4, MPI_LONG_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        int rank = data[0];
        int i = data[1];
        int j = data[2];
        long long val = data[3];

        update_pmat(i, j, val);
    }
}

// Master Works
void master(int world_size){
    const int world_rank = 0;
    init_p();
    print_info(world_size);
    int number = -1;
    
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, recieve_thread, NULL);
    calculate_area(0, world_size, false, true);

    pthread_join(thread_id, NULL);
}

// Slave Works
void slave(int world_size, int world_rank){
    bool is_last = world_rank + 1 >= world_size;
    calculate_area(world_rank, world_size, is_last, false);
}

int main(int argc, char** argv){
    MPI_Init(NULL, NULL);
    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    check_world_size(world_size);

    if(is_master(world_rank)) master(world_size);        
    else                      slave(world_size, world_rank);

    
    MPI_Finalize();
}
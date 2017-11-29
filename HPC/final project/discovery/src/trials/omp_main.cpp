//
//
//Video contains 485 frames. 
//Each frame has dimensions 288x384.

#include <omp.h>
#include <mpi.h>
#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <string.h>
#include <math.h>
#define COL                 288
#define ROW                 384
#define NP                  64
#define TOT_FRAMES          448
#define FRAMES_PER_RANK     TOT_FRAMES/NP
#define THRESHOLD           20

using namespace std; 

int ***video = new int**[TOT_FRAMES];
int ***smoothed_video = new int**[TOT_FRAMES];

double CLOCK() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC,  &t);
    return (t.tv_sec * 1000)+(t.tv_nsec*1e-6);
}

// Parse all image text files with gray matrix values. 
void parse_files(int rank) { 
    string fname;

    int frame_count = 0, x_count = 0, y_count = 0;
    int number_char_count = 0;
    string number = "", exponent = "";
    bool e = false;
    bool sum_char = false;
    double parsed_number = 0;

    // Loop through all files representing frames. 
    video[rank] = new int*[COL];
    smoothed_video[rank] = new int*[COL];
    ifstream file;
    fname = "/home/costarendon.b/project/enter_text/" + to_string(rank+1) + ".txt";
    file.open(fname);
    char c;
    if(!file) {
        cout << "I can't open "<<fname<<"!"<<endl;
        exit(1);
    }
    while(file >> c) {
        if(y_count == 0) {
            video[rank][y_count] = new int[ROW];
            smoothed_video[rank][y_count] = new int[ROW];
        }
        // If you encounter exponent character. 
        if(c == 101)
            e = true;

        if(!e) 
            number += c;

        // If you have encountered exponent and + char. 
        if(e && sum_char)
            exponent += c;

        if(c == 43)
            sum_char = true;

        number_char_count++;
        // If you reach end of number.
        if(number_char_count == 13) {
            parsed_number = stod(number) * pow(10, stoi(exponent));
            video[rank][y_count][x_count] = parsed_number;
            smoothed_video[rank][y_count][x_count] = 0;
            x_count++;
            // If you reach end of row. 
            if(x_count == ROW) {
                x_count = 0;
                y_count++;
                video[rank][y_count] = new int[ROW];
                smoothed_video[rank][y_count] = new int[ROW];
            }
            // If you reach end of column. 
            if(y_count == COL) {
                y_count = 0;
                //cout<<"Frame "<< rank+1 <<" parsed."<<endl;
            } 
            // Reset values for next number. 
            number_char_count = 0;
            number = "";
            exponent = "";
            e = false;
            sum_char = false;
        }
    }
    file.close();
}

// Write matrix to file. 
// Type 1 = Smoothed matrix
// Type 2 = Dt matrix
void write_files(int rank) { 
    ofstream file;
    string fname;
    fname = "/home/costarendon.b/project/filtered_text/" + to_string(rank+1) + ".txt";
    file.open(fname);
    if(file.is_open()) {
        for(int i = 0; i < COL; i++) {
            for(int j = 0; j < ROW; j++) {
                if(rank+1 == FRAMES_PER_RANK)
                    file<<smoothed_video[rank-1][i][j]; 
                else
                    file<<smoothed_video[rank][i][j];
                file<<" ";
            }
            file<<"\n";
        }   
    }
    else 
        cout<<"Unable to open file "<<fname<<endl;
}

// Mask the given frame with a smoothing filter. 
void mask(int frame) {
    int sum = 0;
    for(int col = 1; col < COL-1; col++) {
        for(int row = 1; row < ROW-1; row++) {
            //Apply 3x3 mask. 
            for(int i = -1; i <= 1; i++)
                for(int j = -1; j <= 1; j++)
                    sum += video[frame][col+i][row+j];
            //Average the sum. 
            smoothed_video[frame][col][row] = sum / 9;
            sum = 0;
        }
    }
}

// Calculate temporal difference between frame[i] and frame[i+1].
// Then normalize value depending on threshold. 
void dt_normalize(int frame) {
    for(int col = 0; col < COL; col++) {
        for(int row = 0; row < ROW; row++) {
            smoothed_video[frame][col][row] = abs(smoothed_video[frame+1][col][row] - smoothed_video[frame][col][row]);

            // Normalize significant data. Disregard data otherwise.                 
            if(smoothed_video[frame][col][row] > THRESHOLD)
                smoothed_video[frame][col][row] = 1;
            else
                smoothed_video[frame][col][row] = 0;
        }
    }
}

int main(int argc, char *argv[]) {
    int np, num_threads, nframe = 0, err; 
    
    // MPI variables
    int source, sendcount, recvcount, rank, numtasks;
    double read_start, read_finish, mask_finish, dt_finish, write_finish;    

    // Init MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Init OpenMP
    omp_set_num_threads(FRAMES_PER_RANK);
    num_threads = omp_get_max_threads();

    // Each rank fills FRAMES_PER_RANK in global variables.     
    if(rank == 0) {
        read_start = CLOCK();
        printf("Parsing frames...\n");
    }
    
    #pragma omp parallel
    //for(int i = 0; i < FRAMES_PER_RANK; i++) 
        parse_files(rank*FRAMES_PER_RANK + omp_get_thread_num());

    MPI_Barrier(MPI_COMM_WORLD);

    //-------------------------------------------------------------------//
    // Each MPI process is assigned to mask the number of FRAMES_PER_RANK.
    //  rank 0 = video[0-29]
    //  rank 1 = video[30-59]
    //  rank 2 = video[60-90] ...
    // Smooth the 30 frames with 1/9[1 1 1; 1 1 1; 1 1 1] mask. 
    if(rank == 0) {
        read_finish = CLOCK();
        printf("Masking frames...\n");
    }
    
    #pragma omp parallel
    //for(int frame = 0; frame < FRAMES_PER_RANK; frame++)
        mask(rank*FRAMES_PER_RANK + omp_get_thread_num());

    MPI_Barrier(MPI_COMM_WORLD);

    //----------------------------------------------------------------//
    // Calculate the the temporal derivative with frame[i+1] - frame[i],
    // from frame 0 - frame 29. Frames 30, 60, 90, ... will be removed.  
    if(rank == 0) {
        mask_finish = CLOCK();
        printf("Calculating temporal derivative...\n");
    }

    omp_set_num_threads(FRAMES_PER_RANK-1);

    #pragma omp parallel
    //for(int frame = 0; frame < FRAMES_PER_RANK-1; frame++) 
        dt_normalize(omp_get_thread_num() + rank*FRAMES_PER_RANK);

    omp_set_num_threads(FRAMES_PER_RANK);

    MPI_Barrier(MPI_COMM_WORLD);
    
    //-------------------------------------//
    // Write filtered images into text files.     
    if(rank == 0) {
        dt_finish = CLOCK();
        printf("Writing to files...\n");
    }

    #pragma omp parallel
//    for(int i = 0; i < FRAMES_PER_RANK; i++) 
        write_files(rank*FRAMES_PER_RANK + omp_get_thread_num());

    MPI_Barrier(MPI_COMM_WORLD);
    
    if(rank == 0)
        write_finish = CLOCK();    

    delete[] video;
    delete[] smoothed_video;
    MPI_Finalize();    
    if(rank == 0)  
        printf("Parse time:\t%3.3f\nMask time:\t%3.3f\nDt time:\t%3.3f\nWrite time:\t%3.3f\nTotal time:\t%3.3f\n", read_finish-read_start, mask_finish-read_finish, dt_finish-mask_finish, write_finish-dt_finish, write_finish-read_start);
}

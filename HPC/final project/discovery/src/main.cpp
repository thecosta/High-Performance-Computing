// Motion detection for given frames of video. 
// Usage of MPI to split up the frames into cores. 
// Usage of OpenMP to split up work on frames within each core with 2 threads.
// by Bruno Costa Rendon
//
// Video contains 485 frames. 
// Each frame has dimensions 288x384 pixels.

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

int ***video = new int**[FRAMES_PER_RANK];
int ***smoothed_video = new int**[FRAMES_PER_RANK];

double CLOCK() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC,  &t);
    return (t.tv_sec * 1000)+(t.tv_nsec*1e-6);
}

// Parse all image text files with gray matrix values. 
void parse_files(int thread, int rank) { 
    string fname;

    int frame_count = 0, x_count = 0, y_count = 0;
    int number_char_count = 0;
    string number = "", exponent = "";
    bool e = false;
    bool sum_char = false;
    double parsed_number = 0;
    int n = rank*FRAMES_PER_RANK+thread;

    // Loop through all files representing frames. 
    video[thread] = new int*[COL];
    smoothed_video[thread] = new int*[COL];
    ifstream file;
    fname = "/home/costarendon.b/project/enter_text/" + to_string(n+1) + ".txt";
    file.open(fname);
    char c;
    if(!file) {
        cout << "I can't open "<<fname<<"!"<<endl;
        exit(1);
    }
    while(file >> c) {
        if(y_count == 0) {
            video[thread][y_count] = new int[ROW];
            smoothed_video[thread][y_count] = new int[ROW];
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
            video[thread][y_count][x_count] = parsed_number;
            smoothed_video[thread][y_count][x_count] = 0;
            x_count++;
            // If you reach end of row. 
            if(x_count == ROW) {
                x_count = 0;
                y_count++;
                video[thread][y_count] = new int[ROW];
                smoothed_video[thread][y_count] = new int[ROW];
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
void write_files(int thread, int rank) { 
    ofstream file;
    string fname;
    int n = rank*FRAMES_PER_RANK+thread;
    fname = "/home/costarendon.b/project/filtered_text/" + to_string(n+1) + ".txt";
    file.open(fname);
    if(file.is_open()) {
        for(int i = 0; i < COL; i++) {
            for(int j = 0; j < ROW; j++) {
               // if(thread+1 == FRAMES_PER_RANK)
               //     file<<smoothed_video[thread-1][i][j]; 
               // else
                    file<<smoothed_video[thread][i][j];
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
            if(frame+1 == FRAMES_PER_RANK)
                smoothed_video[frame][col][row] = smoothed_video[frame-1][col][row];
            else
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
    omp_set_num_threads(2);
    num_threads = omp_get_max_threads();

    // Each rank fills FRAMES_PER_RANK in global variables.     
    if(rank == 0) {
        read_start = CLOCK();
        printf("Parsing frames...\n");
    }

    int tile = FRAMES_PER_RANK / num_threads;    

    #pragma omp parallel
    for(int i = omp_get_thread_num() * tile; i < (omp_get_thread_num()+1) * tile; i++) 
        parse_files(i, rank);
    parse_files(FRAMES_PER_RANK-1, rank);

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
    
    for(int frame = 0; frame < FRAMES_PER_RANK; frame++)
        mask(frame);

    //---------------------------------------------------------------//
    // Calculate the temporal derivative with frame[i+1] - frame[i],
    // from frame 0 - frame 29. Frames 30, 60, 90, ... will be copied
    // from previous frame.  
    if(rank == 0) {
        mask_finish = CLOCK();
        printf("Calculating temporal derivative...\n");
    }

    for(int frame = 0; frame < FRAMES_PER_RANK; frame++) 
        dt_normalize(frame);
    //-------------------------------------//
    // Write filtered images into text files.     
    if(rank == 0) {
        dt_finish = CLOCK();
        printf("Writing to files...\n");
    }
    #pragma omp parallel
    for(int i = omp_get_thread_num() * tile; i < (omp_get_thread_num()+1) * tile; i++) 
        write_files(i, rank);
    write_files(FRAMES_PER_RANK-1, rank);

    if(rank == 0)
        write_finish = CLOCK();    

    MPI_Barrier(MPI_COMM_WORLD);

    delete[] video;
    delete[] smoothed_video;
    MPI_Finalize();    
    if(rank == 0)  
        printf("Parse time:\t%3.3f\nMask time:\t%3.3f\nDt time:\t%3.3f\nWrite time:\t%3.3f\nTotal time:\t%3.3f\n", read_finish-read_start, mask_finish-read_finish, dt_finish-mask_finish, write_finish-dt_finish, write_finish-read_start);
}

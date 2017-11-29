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
#define COL             288
#define ROW             384
#define TOT_FRAMES      480
#define NP              16
#define FRAMES_PER_RANK      TOT_FRAMES/NP
#define dtCOL           COL/(int)sqrt(NP)
#define dtROW           ROW/(int)sqrt(NP)
#define FRAME_TILE      4

using namespace std; 
int ***video = new int**[TOT_FRAMES];
int ***smoothed_video = new int**[TOT_FRAMES];

double CLOCK() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC,  &t);
    return (t.tv_sec * 1000)+(t.tv_nsec*1e-6);
}

void checkArr() {
    for(int i = 0; i < TOT_FRAMES; i++)
        for(int j = 0; j < COL; j++)
            for(int k = 0; k < ROW; k++)
                smoothed_video[i][j][k];

    printf("Array test pass.\n");
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
//                cout<<"Frame "<< rank+1 <<" parsed."<<endl;
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

void write_files(int rank) { 
    ofstream file;
    string fname = "/home/costarendon.b/project/filtered_text/" + to_string(rank+1) + ".txt";
    file.open(fname);
    if(file.is_open()) {
        for(int i = 0; i < COL; i++) {
            for(int j = 0; j < ROW; j++) {
                file<<smoothed_video[rank][i][j];
                file<<" ";
            }
            file<<"\n";
        }   
    }
    else 
        cout<<"Unable to open file"<<fname<<endl;
    cout<<"Frame "<<rank<<" written."<<endl;

}

int main(int argc, char *argv[]) {
     
    int np, num_threads, nframe = 0, err; 
    
    // MPI variables
    int source, sendcount, recvcount, rank, numtasks;
    
    // Init MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Init OpenMP
    np = omp_get_num_procs();
    omp_set_num_threads(1);
    num_threads = omp_get_max_threads();

    // Each rank fills FRAMES_PER_RANK in global variables.     
    if(rank == 0)
        printf("Parsing frames...\n");

    for(int i = 0; i < FRAMES_PER_RANK; i++) 
        parse_files(rank*FRAMES_PER_RANK + i);

    MPI_Barrier(MPI_COMM_WORLD);

    // Each MPI process is assigned to mask the number of FRAMES_PER_RANK.
    //  rank 0 = video[0-29]
    //  rank 1 = video[30-59]
    //  rank 2 = video[60-90] ...
    // Smooth the 30 frames with 1/9[1 1 1; 1 1 1; 1 1 1] mask. 
    if(rank == 0)
        printf("Masking frames...\n");

    for(int frame = 0; frame < FRAMES_PER_RANK; frame++) { 
        int sum = 0;
        for(int col = 1; col < COL-1; col++) {
            //printf("rank = %d, v[] = %d\n", rank, rank*FRAMES_PER_RANK + frame);
            for(int row = 1; row < ROW-1; row++) {
                for(int i = -1; i <= 1; i++) 
                    for(int j = -1; j <= 1; j++) 
                        sum += video[rank*FRAMES_PER_RANK + frame][col+i][row+j]; 

            smoothed_video[rank*FRAMES_PER_RANK + frame][col][row] = sum / 9;

            // Print addresses of elements, rows, and frames. 
            /*if(rank == 0 && col == 1 && row == 1) 
                printf("frame = %d, element address = %x, row address = %d, frame address = %d\n", frame, &smoothed_video[rank*FRAMES_PER_RANK + frame][col][row], &smoothed_video[rank*FRAMES_PER_RANK + frame][col], &smoothed_video[rank*FRAMES_PER_RANK + frame]); 
            */
            
            sum = 0;
            }
        }   
    }

    MPI_Barrier(MPI_COMM_WORLD);

    //Send all smoothed frames to rank 0.
/*    if(rank == 0) {
        int*** recv_video = new int**[FRAMES_PER_RANK];
        for(int i = 0; i < FRAMES_PER_RANK; i++) {
            rec_video[i] = new int*[COL];
            for(int j = 0; j < COL; j++) {
                rec_video[i][j] = new int[ROW];
                for(int k = 0; k < ROW; k++) 
                    rec_video[i][j][k] = 0;
            }
        }

        int s = COL*ROW*FRAMES_PER_RANK;
        MPI_Gather(&video+size, size, MPI_INT, &video+size, size, MPI_INT, 0, MPI_COMM_WORLD);
    }  
*/
    //printf("Gathering data from rank %d.\n", rank);
//    int slide = rank*FRAMES_PER_RANK*COL*ROW;
/*    if(rank == 0) {
        for(int i = 0; i < ROW; i++) {
            int **f = smoothed_video[rank*FRAMES_PER_RANK];
            //int *y = f[0];
            //int *x = y[i];
            printf("rank = %d, value = %d, address = %x\n", rank, 1, ((int**)(f) + i));
        }
    }*/

//    MPI_Gather(&video[rank*FRAMES_PER_RANK], slide, MPI_INT, &video+slide, slide, MPI_INT, 0, MPI_COMM_WORLD);
    //Have rank 0 send new copy of smoothed video to all ranks for temporal derivative. 
//    exit(1);

    MPI_Barrier(MPI_COMM_WORLD); 
    //checkArr();
    
    // Temporal derivative of frames. 
    int y, x;
    if(rank == 0)
        printf("Calculating temporal derivative...\n");
//    #pragma omp parallel
    {
        for(int col = 0; col < dtCOL; col++) {
            for(int row = 0; row < dtROW; row++) {
                for(int frame = 1; frame < TOT_FRAMES-1; frame++) {
                    y = (rank / (int)sqrt(NP)) * dtCOL + col;
                    x = (rank % (int)sqrt(NP)) * dtROW + row;
                    //printf("rank =%d, sv = %d\n", rank, smoothed_video[0][x][y]);
                    printf("rank = %d, frame = %d. y = %d, x = %d, smooth = %d\n", rank, frame, y, x, smoothed_video[frame][y][x]);
                    //#pragma omp critical
                    //smoothed_video[frame][y][x] = abs(smoothed_video[frame][y][x] - smoothed_video[frame-1][y][x]);
                }
            }
//            printf("Col %d completed.\n", col);
        }
    }   

    MPI_Barrier(MPI_COMM_WORLD);
    
    // Write filtered images into text files.     
//    #pragma omp parallel 
        for(int i = 0; i < FRAMES_PER_RANK; i++) 
            write_files(rank*FRAMES_PER_RANK + i);

    MPI_Barrier(MPI_COMM_WORLD);

    delete[] video;
    
    MPI_Finalize();      
}

//
//
//Video contains 485 frames. 
//Each frame has dimensions 288x384.

#include <omp.h>
#include <mpi.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <string.h>
#include <math.h>
#define NUM_FRAMES      50
#define FRAME_TILE      4

using namespace std; 
int ***video = new int**[NUM_FRAMES];
int ***smoothed_video = new int**[NUM_FRAMES];
//int **framebuf = new int*[288];

// Parse all image text files with gray matrix values. 
void parse_files() { 
    string fname;

    int frame_count = 0, x_count = 0, y_count = 0;
    int number_char_count = 0;
    string number = "", exponent = "";
    bool e = false;
    bool sum_char = false;
    double parsed_number = 0;

    // Loop through all files representing frames. 
    for(int i = 1; i <= NUM_FRAMES; i++) {
        video[i-1] = new int*[288];
        ifstream file;
        fname = "/home/costarendon.b/project/enter_text/" + to_string(i) + ".txt";
        file.open(fname);
        char c;
        if(!file) {
            cout << "I can't open "<<fname<<"!"<<endl;
            exit(1);
        }
        while(file >> c) {
            if(y_count == 0)
                video[i-1][y_count] = new int[384];
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
                video[i-1][y_count][x_count] = parsed_number;
//                cout<<&video[i-1][y_count][x_count]<<endl;
                x_count++;
                // If you reach end of row. 
                if(x_count == 384) {
                    x_count = 0;
                    y_count++;
                    video[i-1][y_count] = new int[384];
                }
                // If you reach end of column. 
                if(y_count == 288) {
                    y_count = 0;
                    cout<<"Frame "<< i <<" parsed."<<endl;
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
}

/*// Taken from stackoverflow.
// Allocate 2D array. 
int malloc2D(int ***array, int n, int m) {
    int i;
    // allocate the n*m contiguous items 
    int *p = malloc(n*m*sizeof(int));
    if (!p) return -1;

    // allocate the row pointers into the memory 
    (*array) = malloc(n*sizeof(int*));
    if (!(*array)) {
       free(p);
       return -1;
    }

    // set up the pointers into the contiguous memory
    for (i=0; i<n; i++)
       (*array)[i] = &(p[i*m]);

    return 0;
}

// Taken from stackoverflow.
// Deallocate 2D array. 
int free2D(int ***array) {
    // free the memory - the first element of the array is at the start 
    free(&((*array)[0][0]));

    // free the pointers into the memory 
    free(*array);

    return 0;
} */

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
    omp_set_num_threads(np);
    num_threads = omp_get_max_threads();

//    printf("Process %d of %d", rank, numtasks);
//    printf(" with %d threads\n", num_threads);

    // Master thread.
    if(rank == 0) {
        parse_files();
        source = 0;
        sendcount = 288 * 384;
        recvcount = 288 * 384;
    }
    
/*    int** frame;
    malloc2D(&frame, 288 / FRAME_TILE, 384 / FRAME_TILE);

    // Create datatype for frames. 
    int sizes[2] = {288, 384};
    int subsizes[2] = {
    for(int i = 0; i < 288; i++)
        frame[i] = new int[384];

    if(MPI_Scatter(globalptr, sendcount, MPI_INT, frame, recvcount, MPI_INT, source, MPI_COMM_WORLD) != MPI_SUCCESS) {
        printf("Scatter error.\n");
        exit(1);
    }
    printf("Thread %d (0, 0) = %d\n", rank, frame[0][0]); 
    MPI_Barrier(MPI_COMM_WORLD);
    
//    if (MPI_Bcast(&frame, 1, MPI_INT, 0, MPI_COMM_WORLD) != MPI_SUCCESS)
//        exit_on_error(“Error in MPI_Bcast()”);
*/    //delete [] frame;    
    MPI_Finalize();
             
//    smooth_images(); 
//  temporal_derivative();
//  normalize();
    delete [] video;
//    delete [] frame;
}

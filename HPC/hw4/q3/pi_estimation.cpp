//
//Program to compute number PI by taking the ratio of
//random 'darts' thrown with circle area by the total 
//number of 'darts'. This ratio is multiplied by 4.
//Computation done with MPI.  
//by Bruno Costa Rendon
//

#include <stdio.h> // use stdio.h instead of iostream
#include <mpi.h> // mpi header (so we can use MPI_*routines)
#include <stdlib.h> // for rand() and srand()
#include <math.h>  // for sqrt and pow
#include <time.h> // for CLOCK struct

using namespace std;

// Clock function 
double CLOCK() {
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC,  &t);
	return (t.tv_sec * 1000)+(t.tv_nsec*1e-6);
}

int main(int argc, char *argv[]) {
  double ndarts = 1000;
  double circle;
  int numprocs, rank, namelen;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  double start = CLOCK();
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  srand(time(NULL) + rank);
  for(int i = 0; i < ndarts/numprocs; i++) {
    int x = rand() % 11;
    int y = rand() % 11;
    double pos = sqrt(pow((x - 5), 2) + pow((y - 5), 2));
    if(pos <= 5)
      circle++; 
  }

  if(rank == 0) {
    MPI_Status Stat;
    int value;
    for(int source = 1; source < 20; source++) {
      MPI_Recv(&value, 1, MPI_INT, source, 0, MPI_COMM_WORLD, &Stat);
      printf("collected %d from %d\n", value, source);
      circle += value;
      //printf("circle = %lf\n", circle);
    }
    printf("pi aprox: %0.10f\nAccuracy rate: %2.10f%%\n", 4*circle/ndarts, 100*4*circle/ndarts/M_PI);
    double finish = CLOCK(); 
    printf("Time: %2.2fms\n", finish-start);
  }
  else {
    int value = circle; 
    MPI_Send(&value, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    //printf("sent circle %lf...\n", circle);
  }
  MPI_Finalize();
}

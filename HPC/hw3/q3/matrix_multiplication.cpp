//Bruno Costa Rendon
//Matrix multplication of a 1000x1000 matrix by a 1000x1 vector. 

#include <iostream> 
#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <time.h>
#include <vector> 
#define N	1000

using namespace std; 

//Clock function to calculate timing difference. 
double CLOCK() {
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC,  &t);
  return (t.tv_sec * 1000)+(t.tv_nsec*1e-6);
}

//Main function. 
int main() { 
  vector<vector<int> > matrix(N);
  vector<int> victor(N);
  vector<int> result(N);

  ofstream file;
  file.open("results.txt");

  //Initiliaze 1000 x 1000 matrix.
  for(int i = 0; i<N; i++) {
    for(int j = 0; j<N; j++)
	  matrix[i].push_back(rand() % 10 + 1);
  }

  for(int i=0; i<N; i++)
	victor[i] = i;

  double start = CLOCK();
  #pragma ompsq parallel
  {
	#pragma omp single
	{
	  file<<"number of processes: ";
      file<<omp_get_num_procs();
	  file<<"\nnumber of threads: ";
	  file<<omp_get_max_threads();

	  cout<<"number of processes: "<<omp_get_num_procs()<<endl;
      cout<<"number of threads: "<<omp_get_max_threads()<<endl;
	}

	for(int i = 0; i < N; i++) {
	    for(int j = 0; j < N; j++)
	 	  result[i] += matrix[i][j]*victor[j]; 
	}
  } 
  double finish = CLOCK();

  cout<<"Time: "<<finish-start<<endl;
  file<<"\nResult:\n";
  for(int i=0; i<N; i++) {
    file<<result[i];
	file<<"\n";
  }
  file.close();
}

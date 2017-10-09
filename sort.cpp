/*
 * Bruno Costa Rendon
 * EECE5640 High-Performance Computing
 * Homework 1, Problem 2
 * 
 * I implemented the parallel sorting by splitting the array of randomly
 * generated numbers by partitioning it into NT sections, where each part
 * was then assigned to a single thread to sort with an insertion sort
 * algorithm. 
 * Once all the partitions are sorted, I used C++ library algorithm called 
 * inplace_merge, which merges 2 sorted arrays. I initially tried to perform
 * the merging of partitions into more threads, but encountered many issues, 
 * and ended up using the pre-exisiting tool.  
 */

#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <pthread.h>
#include <vector>
#include <math.h>
#include <time.h>
#include <algorithm>
// NT = number of threads. Please select multiple of 2.
// NT must be either 1, 2, 4, or 8 for proper functionality.  
// NN = number of numbers to sort
// NN must be greater than 1000 for proper functionality. 
#define NT	8
#define NN	10000

using namespace std;

// Struct to be used for pthreads.
struct numbers {
  vector<int> digits;
};

// Function to to calculate time elapsed. 
double CLOCK() {
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC,  &t);
  return (t.tv_sec * 1000)+(t.tv_nsec*1e-6);
}

vector<int> total_array;
struct numbers part_array[NT];
struct numbers temp[NT/2];

// Function to read NN randomly generated numbers from local file into
// data structure. 
void init_numbers() { 
  int counter = 0;
  string line;
  ifstream file ("rnum.txt");
  if (file.is_open()) {
    while(getline(file, line)) {
      total_array.push_back(atoi(line.c_str()));
      counter++;
    }
    file.close();
  }
}

// Function to create NN randomly generated numbers, and write them to
// local file. 
void init_ncreation() {
  cout<<"Creating numbers.."<<endl;
  ofstream file;
  file.open("rnum.txt");

  for(int i = 0; i < NN; i++) {
    file << (rand() % NN) + 1;
    file << "\n";
  }
  file.close();
}

// Function to sort a single partition using insertion sort algorithm. 
void* insertion_sort (void *arg) {
  int i, key, j;
  struct numbers *sorted_numbers;
  sorted_numbers = (struct numbers *) arg;
  for (int i = 1; i < NN/NT; i++){
    j = i-1;
    key = sorted_numbers->digits[i];

    while (j >= 0 && sorted_numbers->digits[j] > key) {
      sorted_numbers->digits[j+1] = sorted_numbers->digits[j];
      j--;
    }
    sorted_numbers->digits[j+1] = key;
  }
}

// Function to merge all partitions by every 2 partitions. 
void autoMerge() {
  for(int level = 1; level <= (int)(log2(NT))+1; level=level*2) {
    for(int k = 0; level*(k+2)*NN/NT <= NN; k+=2) {
      int a = level*k*NN/NT, b = level*(k+1)*NN/NT, c = level*(k+2)*NN/NT;
      inplace_merge(total_array.begin()+a, total_array.begin()+b, total_array.begin()+c); 
    }
  }
}

// Main function to call all functions. 
int main(int argc, char *argv[]) {
  cout<<"start"<<endl;
  init_ncreation();
  cout<<"Created random number file."<<endl;
  init_numbers();
  int ret=0; 
  double start, finish;
 
  cout<<"inserting data int partitions.."<<endl;
  // Insert data into partitions. 
  for (int i = 0; i < NT; i++) {
    for (int j = i*NN/NT; j < (i+1)*NN/NT; j++) {
      part_array[i].digits.push_back(total_array[j]);
    }
  }
 
  start = CLOCK();
  pthread_t threads[NT];
  cout<<"sorting each partition..."<<endl;
  // Sort each partition with pthreads. 
  for (int i = 0; i < NT; i++) {
    ret = pthread_create(&threads[i], NULL, insertion_sort, (void *) &part_array[i]);
    pthread_join(threads[i], NULL);
    if (ret) {
      printf("ERROR, return code from pthread_create() is %d\n", ret);
      exit(-1);
    }
  }

  cout<<"inserting all partitions into single vector..."<<endl;
  // Insert all sorted partitions into single vector.
  for (int i=0; i<NT; i++) {
    for(int j=0; j<NN/NT; j++) {
      total_array[j+i*NN/NT] = part_array[i].digits[j];
    }
  }
  
  cout<<"merging partitions..."<<endl;
  // Merge all partitions if there is more than 1 partition. 
  if (NT > 1)
    autoMerge();
 
  finish = CLOCK();
  ofstream file;
  file.open("results.txt", fstream::app);
  double total_time = finish - start; 
  file<<"Threads: "<<NT<<", array size: "<<NN<<", time: "<<total_time<<"ms\n";
  file.close();

  ofstream file2;
  file2.open("sorted_numbers.txt");
  for(int i =0; i <NN; i++) 
    file2<<total_array[i]<<"\n";
  file2.close();

  pthread_exit(NULL);
  return 0;
}

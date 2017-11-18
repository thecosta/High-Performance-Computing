/* Bruno Costa Rendon
   Homework 2
   Philosophers and Forks
   To manage all my threads I do:
     (1) create N threads, one for each philosopher.
     (2) each philosopher is put into a priority queue.
     (3) if it's the philosophers priority turn, and the the philosopher can
         eat, then they are removed from the priority queue, and broadcasts
         to the rest of the philosophers about their thinking initiation.
     (4) if the philosopher can eat, but it is not his prority turn, then they
         will wait on the priority philosopher to begin thinking and eating.
*/

#include <iostream>
#include <pthread.h>
#include <vector>
#include <algorithm>
#include <fstream>
#include <queue>
#define NN		10000000
using namespace std;

pthread_mutex_t bon_apetit;
pthread_cond_t iphil;
vector<int> nums;
queue<int> serving_priority;
bool first_serving;

// Table of philosophers.
// Contains the indexing of philosophers, as well as forks.
// Also contains the size of table.
struct table {
  vector<int> pos;
  int size;
} table1; 

// Create file with random numberds. 
void init_ncreation() {
  ofstream file;
  file.open("rnum.txt");

  for(int i = 0; i < NN; i++) {
    file << (rand() % NN) + 1;
    file << "\n";
  }
  file.close();
}

// Starting function to create material for philosophers to reflect on and sort.
void read_nums() {
  int counter = 0;
  vector<int> nums;
  string line;
  ifstream file ("rnum.txt");
  if (file.is_open()) {
    while(getline(file, line)) {
      nums.push_back(atoi(line.c_str()));
      counter++;
    }
    file.close();
  }
}

// Status of the table. Prints who is/isn't eating, and which forks
// are/aren't being used.
void status(table *arr) {
  for(int i = 0; i < arr->size; i++) {
    if (arr->pos[i] == 1)
      cout<<"       | ";
    if (arr->pos[i] == 0)
      cout<<"       | ";
    if (arr->pos[i] == -1)
      cout<<"   X   | ";
    if (arr->pos[i] == 2)
      cout<<" Yum!  | ";
  }
  cout<<endl;
}

// Given philosopher at index n, can does he have the necessary forks
// to eat and think?
bool are_forks_available(int n) {
  if(n == 0) {
    if(table1.pos[n+1] == 0 && table1.pos[table1.size-1] == 0) {
      table1.pos[n+1] = -1;
      table1.pos[table1.size-1] = -1;
      table1.pos[n] = 2;
	  return true;
    }
  }
  else {
    if(table1.pos[n-1] == 0 && table1.pos[n+1] == 0) {
      table1.pos[n-1] = -1;
      table1.pos[n+1] = -1;
      table1.pos[n] = 2;
      return true;
    }
  }
  return false;
}

// The philosopher n has finished eating and thinking! Time to clean up.
void cleanup_forks(int n) {
  if(n == 0) {
    if(table1.pos[n+1] == -1 && table1.pos[table1.size-1] == -1) {
      table1.pos[n+1] = 0;
      table1.pos[table1.size-1] = 0;
      table1.pos[n] = 1;
    }
  }
  else {
    if(table1.pos[n-1] == -1 && table1.pos[n+1] == -1) {
      table1.pos[n-1] = 0;
      table1.pos[n+1] = 0;
      table1.pos[n] = 1;
    }
  }
}

// Can this philosopher think and eat without interference from adjacent
// philosophers?
bool to_be_not_to_be(int n) {
  if(n == 0) {
    if(table1.pos[n+2] == 1 && table1.pos[table1.size-2] == 1) {
	  return true;
    }
  }
  if(n == table1.size-2) {
	if(table1.pos[0] == 1 && table1.pos[table1.size-2] == 1) {
      return true;
    }
  }
  else {
    if(table1.pos[n+2] == 1 && table1.pos[n-2] == 1) {
	  return true;
    }
  }
  return false;
}

// If all conditions are met, may the philosopher eat and think. 
void *think(void *threadid) {
  int phil = (long) threadid;
  phil = phil*2;

  if(!to_be_not_to_be(phil)) 
	pthread_exit(NULL);

  if(!first_serving) {
    if(serving_priority.front() != phil) 
	  pthread_cond_wait(&iphil, &bon_apetit);
    else {
	  pthread_mutex_lock(&bon_apetit);
	  first_serving = true;
	  serving_priority.pop();
	  pthread_cond_broadcast(&iphil);
    }
  }

  if(are_forks_available(phil)) {
	read_nums();
    sort(nums.begin(), nums.end());
  }
  
  pthread_mutex_unlock(&bon_apetit);
  pthread_exit(NULL);
}

// Main function to manage the table of philosophers, forever eating
// and thinking. 
int main() { 
  int in, count = 0;
  cout<<"How many philosophers do you need? ";
  cin>>in;
  void *state;
  
  table1.size = in*2;
  pthread_t threads[in];
  pthread_t tmanager;
  pthread_cond_init(&iphil, NULL);
  pthread_mutex_init(&bon_apetit, NULL);
  init_ncreation();
 
  for(int i=0; i<in*2; i++) {
	if (i % 2) { 
	  table1.pos.push_back(0);			    //0 represents a fork. No fork = -1. 
	  cout<<"Fork "<<i/2<<" | ";
    }
	else {
	  table1.pos.push_back(1);			    //1 represents an idle philospher. Thinking philosopher = 2.
	  cout<<"Phil "<<i/2<<" | ";
    }
	if(i+1 == in*2)
	  cout<<endl;
  }

  status(&table1);

  while(1) {
	pthread_mutex_t bon_apetit;
	pthread_cond_t iphil;
	first_serving = false;

	for(long k = 0; k < in; k++) {
	  pthread_create(&threads[k], NULL, think, (void *)k);
	  serving_priority.push(k*2);
	}
	
	pthread_mutex_lock(&bon_apetit);

	for (long k = 0; k < in; k++) {
	  pthread_join(threads[k], NULL);
	}

	status(&table1);

	for(int k = 0; k < table1.size; k++) {
      if(table1.pos[k] == 2) {
        cleanup_forks(k);
	  }
    }

	status(&table1); 

	pthread_mutex_unlock(&bon_apetit);
  }
  return 0;
}

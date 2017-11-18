#include <iostream>
#include <pthread.h>
#include <vector>
#include <math.h>
#include <time.h>

using namespace std; 

pthread_mutex_t mutex_t;
int next_prime;
int cap;
vector<int> final_primes;

struct numbers {
  vector<int> nprime;
  int partitions;
} numbers1;

double CLOCK() {
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC,  &t);
  return (t.tv_sec * 1000)+(t.tv_nsec*1e-6);
}

void update_next_prime() {
  for(int i=next_prime; i < numbers1.nprime.size(); i++) {
	if(numbers1.nprime[i] > 0 && numbers1.nprime[i] > next_prime) {
	  next_prime = numbers1.nprime[i];
	  return;
	}
  }
  return;
}

void *mark(void *threadid) {
  int t = (long) threadid;
  int start = t * numbers1.nprime.size()/numbers1.partitions;
  int end = (t+1) * numbers1.nprime.size() / numbers1.partitions;

  pthread_mutex_lock(&mutex_t);

  for(int k=start; k < end; k++) {
	if(numbers1.nprime[k] % next_prime == 0 && numbers1.nprime[k] > 0 && numbers1.nprime[k] != next_prime)
	  numbers1.nprime[k] = numbers1.nprime[k]*(-1);
  }

  pthread_mutex_unlock(&mutex_t);
  
  pthread_exit(NULL);
}

void clean_up() {
  for(int k = 0; k < numbers1.nprime.size(); k++) {
	if(numbers1.nprime[k] < 0 || numbers1.nprime[k] == 1) {
	  numbers1.nprime.erase(numbers1.nprime.begin()+k);
	  k--;
	}
  }
}

int main() {
  int prime;
  int t;
  cout<<"Largest prime: ";
  cin>>prime;
  cout<<"Number of threads: ";
  cin>>t;
  cap = floor(sqrt(prime));
  double start, finish;

  pthread_mutex_t mutex_t;
  pthread_t threads[t];
  pthread_mutex_init(&mutex_t, NULL);

  numbers1.partitions = t;
  next_prime = 2;
  final_primes.push_back(next_prime);

  for(int i=1; i <= prime; i++) 
	numbers1.nprime.push_back(i);

  start = CLOCK();
  while(next_prime <= cap) {
    for(long k=0; k < t; k++) {
      pthread_create(&threads[k], NULL, mark, (void *)k);
	}
  
    pthread_mutex_lock(&mutex_t);
  
    for(int k=0; k < t; k++) 
	  pthread_join(threads[k], NULL);

    pthread_mutex_unlock(&mutex_t);
	if(final_primes[final_primes.size()-1]  != next_prime)
	  final_primes.push_back(next_prime);
	update_next_prime();
  }
  clean_up();
  finish = CLOCK();
  for(int i = 0; i<numbers1.nprime.size(); i++)
    cout<<numbers1.nprime[i]<<endl;
  cout<<"Total time: "<<finish-start<<endl;
  cout<<numbers1.nprime.size()<<endl;
}
#include <algorithm>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <omp.h>
	
using namespace std; 

vector<vector<double> > matrix;
vector<vector<double> > lower; 
vector<vector<double> > upper;
vector<vector<double> > identity;
vector<vector<double> > forward;
vector<vector<double> > inverse;
int bsize = 4;

double CLOCK() {
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC,  &t);
	return (t.tv_sec * 1000)+(t.tv_nsec*1e-6);
}

void forward_sub(int size) { 
  for(int k = 0; k < size; k++) {
		for(int i = 0; i < size; i++) {
			double sum =0;			
			//#pragma omp parallel for reduction(+: sum)
				for(int j = 0; j < i; j++) 
					sum += lower[i][j] * forward[j][k];
			forward[i][k] = identity[i][k] - sum; 
		}
	}
}

void backward_sub(int size) {
  for(int k = size -1; k >= 0; k--) {
    for(int i = size -1; i >= 0; i--) {
      double sum = 0;
			//#pragma omp parallel for reduction(+: sum)	
     		for(int j = size -1; j > i; j--) 
        	sum += upper[i][j] * inverse[j][k];
      inverse[i][k] = (forward[i][k] - sum) / upper[i][i];
    }
  }
}

void print_status(int size) { 
	cout<<"Spawned Matrix: "<<endl;
  for(int i = 0; i < size; i++) {
    for(int j = 0; j < size; j++)
      cout<<matrix[i][j]<<" ";
    cout<<endl;
  }
  cout<<"Lower Matrix: "<<endl;
  for(int i = 0; i < size; i++) {
		for(int j = 0; j < size; j++) 
			cout<<lower[i][j]<<" ";
		cout<<endl;
	}
	cout<<"Upper Matrix: "<<endl;
	for(int i = 0; i < size; i++) {
		for(int j = 0; j < size; j++)
			cout<<upper[i][j]<<" ";
		cout<<endl;
	}
  cout<<"Forward Substitution: "<<endl;
  for(int i = 0; i < size; i++) {
    for(int j = 0; j < size; j++)
      cout<<forward[i][j]<<" ";
    cout<<endl;
  }
	cout<<"Inverse Matrix: "<<endl;
  for(int i = 0; i < size; i++) {
    for(int j = 0; j < size; j++)
      cout<<inverse[i][j]<<" ";
    cout<<endl;
  }
}

//LU decomposition taken from geeksforgeeks.org . 
void LU_decomposition(int size) { 
  // Decomposing matrix into Upper and Lower
  // triangular matrix
//	#pragma omp parallel for
  for (int i = 0; i < size; i++) {
  	// Upper Triangular
		for (int k = i; k < size; k++) {
	  	// Summation of L(i, j) * U(j, k)
			double sum = 0;
//		  #pragma omp parallel for reduction(+: sum)
				for (int j = 0; j < i; j++)
		  		sum += (lower[i][j] * upper[j][k]);
				// Evaluating U(i, k)
				upper[i][k] = matrix[i][k] - sum;
		}						 
		// Lower Triangular
		for (int k = i; k < size; k++) {
			if (i == k)
				lower[i][i] = 1; // Diagonal as 1
			else {
				// Summation of L(k, j) * U(j, i)
				double sum = 0;
					for (int j = 0; j < i; j++)
						sum += (lower[k][j] * upper[j][i]);
				// Evaluating L(k, i)
				lower[k][i] = (matrix[k][i] - sum) / upper[i][i];
			}
		}
	}
}

void manual_init(int size) { 
	cout<<"Enter matrix: ";
	for(int i = 0; i < size; i++) 
		for(int j = 0; j < size; j++) 
			cin>>matrix[i][j];
} 

void random_init(int size) {
  for(int i = 0; i<size; i++) { 
    vector<double> temp(size);
  	vector<double> zeros(size);
		vector<double> itemp(size);
    for(int j = 0; j<size; j++) {
      if(i == j) {
				temp.at(j) = 40;
				itemp.at(j) = 1;
			}
      else {
        temp.at(j) = rand() % 20;
				itemp.at(j) = 0;
			}
			zeros.at(j) = 0;
    }
		identity.push_back(itemp);
    matrix.push_back(temp);
		lower.push_back(zeros);
		upper.push_back(zeros);
		forward.push_back(itemp);
		inverse.push_back(itemp);
  }
}

int main() { 
  int size, select;
  cout<<"Select option for matrix generation:\n\t(1) Random generation.\n\t(2) Manual entry."<<endl;
	cin>>select;
  cout<<"Enter length of matrix: ";
  double start, finish;
  cin >> size; 
  random_init(size);
	if(select == 2)
		manual_init(size);
  start = CLOCK();
  LU_decomposition(size);
//	#pragma omp parallel 
	{
	forward_sub(size);
	backward_sub(size);
	}
	finish = CLOCK();
	cout<<"Print results? 1 = yes, 0 = no"<<endl;
	int p;
	cin>>p;
	if (p)
		print_status(size);	
  cout<<"Time: "<<finish-start<<endl;
}
    
    

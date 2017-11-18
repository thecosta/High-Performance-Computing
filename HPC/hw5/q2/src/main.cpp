// Main file to prepare data to be sent to GPU.  
// by Bruno Costa Rendon
//
// argv[1] = input image
// argv[2] = output image
// argv[3] = lower histogram bound
// argv[4] = upper histogram bound

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <ctime>

using namespace cv;
using namespace std;

extern void classify(unsigned char *in_mat,
		       unsigned char *hist_image,
                       int *histogram,
                       int lower, int upper,
		       unsigned int height,
		       unsigned int width);

double CLOCK() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC,  &t);
    return (t.tv_sec * 1000)+(t.tv_nsec*1e-6);
}

int main(int argc, const char** argv) { 
  Mat input_image = imread(argv[1], IMREAD_GRAYSCALE);
  double start_gpu, finish_gpu; 
  int lower =(int) atoi(argv[3]);
  int upper =(int) atoi(argv[4]);
  int *histogram = new int[upper-lower]();

  if (input_image.empty()){
    cout << "Image cannot be loaded..!!" << endl;
    return -1;
  }

  unsigned int height = input_image.rows;
  unsigned int width = input_image.cols;

  // Data to be passed to GPU. 
  // GPU will return hist_image with result. 
  Mat hist_image = Mat::zeros(height, width, CV_8U);
  start_gpu = CLOCK();   

  classify((unsigned char *)input_image.data, 
                    (unsigned char *)hist_image.data,
	            histogram, 	 
                    lower, upper,
                    height, width);
 
  finish_gpu = CLOCK();

  cout << "GPU execution time: " << finish_gpu - start_gpu << " ms" << endl;
  cout << "writing output image " << argv[2] << endl;

  imwrite (argv[2], hist_image);
    
  int largest = 0;
  for(int i = 0; i < upper-lower; i++) 
    if(histogram[i] > largest) 
        largest = histogram[i];

  //Print out histogram on console.
  printf("w = %d, h = %d\n", width, height);
  printf("Color\tCount\t\tColor count in image\n");
  for(int i = 0; i <= upper-lower; i++) {
    printf("%d\t%d\t", lower+i, histogram[i]);
    for(int j = 0; j < 50*histogram[i]/largest; j++)
        printf("=");
    printf("\n");
  }

  return 0;  

}

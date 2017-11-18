// Find pixels within histogram range specified by user. 
// Add to gray color's count value atomically, and filter 
// out pixels not within histogram range. 
// by Bruno Costa Rendon

#include <stdio.h>
#include <stdlib.h>
#include <cuda.h>
#include <time.h>
 
#define TIMER_CREATE(t)               \
  cudaEvent_t t##_start, t##_end;     \
  cudaEventCreate(&t##_start);        \
  cudaEventCreate(&t##_end);


#define TIMER_START(t)                \
  cudaEventRecord(t##_start);         \
  cudaEventSynchronize(t##_start);    \


#define TIMER_END(t)                             \
  cudaEventRecord(t##_end);                      \
  cudaEventSynchronize(t##_end);                 \
  cudaEventElapsedTime(&t, t##_start, t##_end);  \
  cudaEventDestroy(t##_start);                   \
  cudaEventDestroy(t##_end);

#define TILE_SIZE 3
#define CUDA_TIMING

unsigned char *input_gpu;
unsigned char *output_gpu;
int *out_histogram;

/*******************************************************/
/*                 Cuda Error Function                 */
/*******************************************************/
inline cudaError_t checkCuda(cudaError_t result) {
    #if defined(DEBUG) || defined(_DEBUG)
    if (result != cudaSuccess) {
        fprintf(stderr, "CUDA Runtime Error: %s\n", cudaGetErrorString(result));
        exit(-1);
    }
    #endif
    return result;
}

// GPU kernel and functions
__global__ void kernel(unsigned char *input,
                       unsigned char *hist_image,
                       int* histogram,
                       int lower, int upper,
                       unsigned int height,
                       unsigned int width) {

    int x = blockIdx.x*TILE_SIZE+threadIdx.x;
    int y = blockIdx.y*TILE_SIZE+threadIdx.y;
    int index = x + y * width;

    if (x < width && y < height) {
        // If pixel is within histogram range. 
        if(input[index] >= lower && input[index] <= upper) {
            atomicAdd(&histogram[input[index]-lower], 1);
            hist_image[index] = input[index];
        }
        else 
            hist_image[index] = 0;
    }

    __syncthreads();
}

void classify(unsigned char *int_mat, 
	       unsigned char *hist_image, 
               int* histogram,
               int lower, int upper,
	       unsigned int height, 
	       unsigned int width) {
    int gridXSize = 1 + (( width - 1) / TILE_SIZE);
    int gridYSize = 1 + ((height - 1) / TILE_SIZE);

    int XSize = gridXSize*TILE_SIZE;
    int YSize = gridYSize*TILE_SIZE;

    // Both are the same size (CPU/GPU).
    int size = XSize*YSize;
    int hist_size = upper-lower+1;

    // Allocate arrays in GPU memory
    checkCuda(cudaMalloc((void**)&input_gpu, size*sizeof(unsigned char)));
    checkCuda(cudaMalloc((void**)&output_gpu, size*sizeof(unsigned char)));
    checkCuda(cudaMalloc((void**)&out_histogram, hist_size*sizeof(int)));

    checkCuda(cudaMemset(output_gpu, 0, size*sizeof(unsigned char)));
    checkCuda(cudaMemset(out_histogram, 0, hist_size*sizeof(int)));

    // Copy data to GPU
    checkCuda(cudaMemcpy(input_gpu,
                        int_mat,
                        height*width*sizeof(char),
                        cudaMemcpyHostToDevice));
    
    // Wait for all threads to synchronize
    checkCuda(cudaDeviceSynchronize());

     // Execute algorithm
    dim3 dimGrid(gridXSize, gridYSize);
    dim3 dimBlock(TILE_SIZE, TILE_SIZE);

    printf("All memory allocated and set.\n");    

    #if defined(CUDA_TIMING)
        float Ktime;
        TIMER_CREATE(Ktime);
        TIMER_START(Ktime);
    #endif

    // Kernel Call
    kernel<<<dimGrid, dimBlock>>>(input_gpu, output_gpu, out_histogram, lower, upper, height, width);

    checkCuda(cudaDeviceSynchronize());

    #if defined(CUDA_TIMING)
        TIMER_END(Ktime);
        printf("Kernel Execution Time: %f ms\n", Ktime);
    #endif

    // Retrieve results from the GPU
    checkCuda(cudaMemcpy(hist_image,
                        output_gpu,
                        height*width*sizeof(unsigned char),
                        cudaMemcpyDeviceToHost));
    checkCuda(cudaMemcpy(histogram, 
                        out_histogram, 
                        hist_size*sizeof(int), 
                        cudaMemcpyDeviceToHost));

    // Free resources and end the program
    checkCuda(cudaFree(output_gpu));
    checkCuda(cudaFree(input_gpu));
    checkCuda(cudaFree(out_histogram));    
}

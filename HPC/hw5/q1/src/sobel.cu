// Apply Sobel mask to grayed image. 
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
                       unsigned char *output,
//                       unsigned char *mask_x,
//                       unsigned char *mask_y,
                       unsigned int height,
                       unsigned int width) {

    int x = blockIdx.x*TILE_SIZE+threadIdx.x;
    int y = blockIdx.y*TILE_SIZE+threadIdx.y;
    int index = x + y * width;
    
    int sobel_x[3][3] = {{1, 0, -1}, {2, 0, -2}, {1, 0, -1}};
    int sobel_y[3][3] = {{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};
    
    if (x < width-1 && y < height-1  && x > 0 && y > 0) {
        int sum_x = 0;
        int sum_y = 0;
        //printf("(1) sum_x = %d, sum_y = %d\n", sum_x, sum_y); 
//        printf("%d\n", mask_x[0]);
        for(int i = -1; i <= 1; i++) {
            for(int j = -1; j <=1; j++) {
                sum_x += input[(x+j) + (y+i)*width] * sobel_x[i+1][j+1];
                sum_y += input[(x+j) + (y+i)*width] * sobel_y[i+1][j+1];
            }
        }
//        printf("sum_x = %d, sum_y = %d\n", sum_x, sum_y);
        output[index] = sqrtf(sum_x*sum_x + sum_y*sum_y);
    //    printf("in loop");
    //printf("output[%d] = ", x*width+y);
    }
    __syncthreads();
}

void sobel_img(unsigned char *int_mat, 
	       unsigned char *out_mat, 
//               unsigned char *mask_x,
//               unsigned char *mask_y,
	       unsigned int height, 
	       unsigned int width) {
    printf("w = %d, h = %d\n", width, height);
    int gridXSize = 1 + (( width - 1) / TILE_SIZE);
    int gridYSize = 1 + ((height - 1) / TILE_SIZE);

    int XSize = gridXSize*TILE_SIZE;
    int YSize = gridYSize*TILE_SIZE;

    // Both are the same size (CPU/GPU).
    int size = XSize*YSize;

    // Allocate arrays in GPU memory
    checkCuda(cudaMalloc((void**)&input_gpu, size*sizeof(unsigned char)));
    checkCuda(cudaMalloc((void**)&output_gpu, size*sizeof(unsigned char)));

    checkCuda(cudaMemset(output_gpu , 0 , size*sizeof(unsigned char)));

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

    #if defined(CUDA_TIMING)
        float Ktime;
        TIMER_CREATE(Ktime);
        TIMER_START(Ktime);
    #endif

    // Kernel Call
    kernel<<<dimGrid, dimBlock>>>(input_gpu, output_gpu, height, width);

    checkCuda(cudaDeviceSynchronize());

    #if defined(CUDA_TIMING)
        TIMER_END(Ktime);
        printf("Kernel Execution Time: %f ms\n", Ktime);
    #endif

    // Retrieve results from the GPU
    checkCuda(cudaMemcpy(out_mat,
                        output_gpu,
                        height*width*sizeof(unsigned char),
                        cudaMemcpyDeviceToHost));

    // Free resources and end the program
    checkCuda(cudaFree(output_gpu));
    checkCuda(cudaFree(input_gpu));    
}

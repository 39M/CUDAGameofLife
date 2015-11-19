
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include <assert.h>
#include <algorithm>

#include "pre_define.h"

#define CUDA_CALL(__func__) {const cudaError_t __cuda_err__ = (__func__); if (__cuda_err__ != cudaSuccess) {printf("\nCuda Error: %s (err_num=%d)\n", cudaGetErrorString(__cuda_err__), __cuda_err__); cudaDeviceReset(); assert(0);}}

/********** Version 1 **********/
// Use symbol(__device__ variables as below) to store cells status


__device__ char gpu_cells[CELL_X + 2][CELL_Y + 2];
__device__ char gpu_cells_next[CELL_X + 2][CELL_Y + 2];


// Kernel
__global__ void simpleUpdateKernel()
{
	int i = blockIdx.x + 1;
	int j = threadIdx.x + 1;
	int cellsCount = gpu_cells[i - 1][j - 1] + gpu_cells[i - 1][j] + gpu_cells[i - 1][j + 1] +
		gpu_cells[i][j - 1] + gpu_cells[i][j + 1] +
		gpu_cells[i + 1][j - 1] + gpu_cells[i + 1][j] + gpu_cells[i + 1][j + 1];

	if (cellsCount == 3)
		gpu_cells_next[i][j] = 1;
	else if (cellsCount == 2)
		gpu_cells_next[i][j] = gpu_cells[i][j];
	else
		gpu_cells_next[i][j] = 0;
}


// A not efficient kernel
/*__global__ void updateKernelPlus()
{
	int i = blockIdx.x / (CELL_Y / BLOCK_DIM) + 1;
	int j = threadIdx.x + BLOCK_DIM * (blockIdx.x % (CELL_Y / BLOCK_DIM)) + 1;
	int cellsCount = gpu_cells[i - 1][j - 1] + gpu_cells[i - 1][j] + gpu_cells[i - 1][j + 1] +
		gpu_cells[i][j - 1] + gpu_cells[i][j + 1] +
		gpu_cells[i + 1][j - 1] + gpu_cells[i + 1][j] + gpu_cells[i + 1][j + 1];

	if (cellsCount == 3)
		gpu_cells_next[i][j] = 1;
	else if (cellsCount == 2)
		gpu_cells_next[i][j] = gpu_cells[i][j];
	else
		gpu_cells_next[i][j] = 0;
}*/


// Copy CPU cells status to GPU and call kernel, then copy data back
extern "C" int CUDAUpdate(char cells[CELL_X + 2][CELL_Y + 2], int iterateTime)
{
	CUDA_CALL(cudaMemcpyToSymbol(gpu_cells, cells, (CELL_X + 2) * (CELL_Y + 2)));
	for (int iterator = 0; iterator < iterateTime; iterator++)
	{
		simpleUpdateKernel << <CELL_X, CELL_Y >> >();
		//updateKernelPlus << < CELL_X * (CELL_Y / BLOCK_DIM), BLOCK_DIM >> >();

		// Why not copy data form GPU to GPU directly?
		// But there's not a function "cudaMemcpyFromSymbolToSymbol"
		// This makes it less efficient
		CUDA_CALL(cudaMemcpyFromSymbol(cells, gpu_cells_next, (CELL_X + 2) * (CELL_Y + 2)));
		CUDA_CALL(cudaMemcpyToSymbol(gpu_cells, cells, (CELL_X + 2) * (CELL_Y + 2)));

		// This function cannot use for symbol
		//CUDA_CALL(cudaMemcpy(gpu_cells, gpu_cells_next, (CELL_X + 2) * (CELL_Y + 2), cudaMemcpyDeviceToDevice));
	}
	CUDA_CALL(cudaMemcpyFromSymbol(cells, gpu_cells, (CELL_X + 2) * (CELL_Y + 2)));
	return 0;
}
/********** Version 1 End **********/


/********** Version 2 **********/
// Alloc GPU memory to store cells status


// Kernel
__global__ void anotherSimpleUpdateKernel(char *gpu_cells, char *gpu_cells_next)
{
	int i = blockIdx.x + 1;
	int j = threadIdx.x + 1;
	int cellsCount = gpu_cells[(i - 1) * (CELL_Y + 2) + j - 1] + gpu_cells[(i - 1) * (CELL_Y + 2) + j] + gpu_cells[(i - 1) * (CELL_Y + 2) + j + 1] +
		gpu_cells[i * (CELL_Y + 2) + j - 1] + gpu_cells[i * (CELL_Y + 2) + j + 1] +
		gpu_cells[(i + 1) * (CELL_Y + 2) + j - 1] + gpu_cells[(i + 1) * (CELL_Y + 2) + j] + gpu_cells[(i + 1) * (CELL_Y + 2) + j + 1];

	if (cellsCount == 3)
		gpu_cells_next[i * (CELL_Y + 2) + j] = 1;
	else if (cellsCount == 2)
		gpu_cells_next[i * (CELL_Y + 2) + j] = gpu_cells[i * (CELL_Y + 2) + j];
	else
		gpu_cells_next[i * (CELL_Y + 2) + j] = 0;
}


// Copy CPU cells status to GPU and call kernel, then copy data back
extern "C" int anotherCUDAUpdate(char cells[CELL_X + 2][CELL_Y + 2], int iterateTime)
{
	char *gpu_cells_pointer;
	char *gpu_cells_next_pointer;

	CUDA_CALL(cudaMalloc((void**)&gpu_cells_pointer, (CELL_X + 2) * (CELL_Y + 2)));
	CUDA_CALL(cudaMalloc((void**)&gpu_cells_next_pointer, (CELL_X + 2) * (CELL_Y + 2)));

	CUDA_CALL(cudaMemcpy(gpu_cells_pointer, cells, (CELL_X + 2) * (CELL_Y + 2), cudaMemcpyHostToDevice));
	CUDA_CALL(cudaMemset(gpu_cells_next_pointer, 0, (CELL_X + 2) * (CELL_Y + 2)));

	for (int iterator = 0; iterator < iterateTime; iterator++)
	{
		anotherSimpleUpdateKernel << <CELL_X, CELL_Y >> >(gpu_cells_pointer, gpu_cells_next_pointer);
		//CUDA_CALL(cudaMemcpy(gpu_cells_pointer, gpu_cells_next_pointer, (CELL_X + 2) * (CELL_Y + 2), cudaMemcpyDeviceToDevice));
		std::swap(gpu_cells_pointer, gpu_cells_next_pointer);
	}
	CUDA_CALL(cudaMemcpy(cells, gpu_cells_pointer, (CELL_X + 2) * (CELL_Y + 2), cudaMemcpyDeviceToHost));

	CUDA_CALL(cudaFree(gpu_cells_pointer));
	CUDA_CALL(cudaFree(gpu_cells_next_pointer));
	return 0;
}
/********** Version 2 End **********/

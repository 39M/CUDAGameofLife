/* Use for connect CUDA kernel function with normal cpp project */

extern "C" int CUDAUpdate(char cells[CELL_X + 2][CELL_Y + 2], int iterateTime);
extern "C" int anotherCUDAUpdate(char cells[CELL_X + 2][CELL_Y + 2], int iterateTime);

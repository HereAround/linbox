/*
 * kernel_partial_8_dp.cl
 *
 *  Created on: Jul 5, 2011
 *      Author: Matthew Wezowicz
 */

#define BLOCK_SIZE 16
#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void matrixMulKernelModular8DP(__global double* C, __global double* A, __global double* B,
		const int widthA, const int widthB, const double mod){
	//Get Workgroup ID
	int bx = get_group_id(0);
	int by = get_group_id(1);

	//Get Local ID
	int tx = get_local_id(0);
	int ty = get_local_id(1);

	//Range of indecies for sub-matrix of A
	int aBegin = widthA * BLOCK_SIZE * by;
	int aEnd = aBegin + widthA - 1;
	int aStep = BLOCK_SIZE;

	//Range of indecies for sub-matrix of B
	int bBegin = BLOCK_SIZE * bx;
	int bStep = BLOCK_SIZE * widthB;

	//Local storage of sub-matrices of A and B
	__local double As[BLOCK_SIZE][BLOCK_SIZE];
	__local double Bs[BLOCK_SIZE][BLOCK_SIZE];

	//Temporary storage for result
	double Csub = 0;

	//Loop over all the sub-matrices of A and B required to compute
	//the result sub-matrix
	for(int a = aBegin, b = bBegin; a < aEnd; a += aStep, b += bStep){
		//Load the matrices from global memory to local memory
		//Each thread loads one element of each sub-matrix
		As[ty][tx] = A[a + widthA * ty + tx];
		Bs[ty][tx] = B[b + widthB * ty + tx];

		//Synchronize threads
		barrier(CLK_LOCAL_MEM_FENCE);

		//Multiply the two sub-matrices together
		for(int i = 0; i < BLOCK_SIZE / 2; i++){
			Csub += As[ty][i] * Bs[i][tx];
		}
		Csub = fmod(Csub, mod);

		for(int i = BLOCK_SIZE / 2; i < BLOCK_SIZE; i++){
			Csub += As[ty][i] * Bs[i][tx];
		}
		Csub = fmod(Csub, mod);

		//Synchronize threads
		barrier(CLK_LOCAL_MEM_FENCE);
	}

	//Calculates the offset in the result matrix and add the sum to the
	//appropriate spot
	int c = widthB * BLOCK_SIZE * by + BLOCK_SIZE * bx;
	C[c + ty * widthB + tx] = Csub;
}

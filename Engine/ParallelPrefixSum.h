#pragma once
#include <vector>


typedef unsigned int UINT;


static void prefixSumParallel(std::vector<UINT>& dataset)
{
	// d is num threads in use at any one time...
	// this algorithm uses a varying number of threads at a time, and probably works better on GPUs, for 4 threads im not sure
	// if it's worth complicating it so much as sequential prefix sum over 8160 elements is pretty fast in release

	int n = 8192;	// 8160 but pad it (this was supposed to be for clustered shading)
	int log2n = log2(n);

	for (int d = 0; d < log2n; d++)
	{
		for (int k = 0; k < n; k += (2 << d))		//pow(2, d + 1) = 2 << d
		{
			dataset[k + (2 << d) - 1] = dataset[k + (2 << (d - 1)) - 1] + dataset[k + (2 << d) - 1];
		}
	}

	dataset.back() = 0;	// DO THIS BEFORE THE PARALLEL INVOCATION!

	for (int d = log2n - 1; d >= 0; d--)
	{
		for (int k = 0; k < n; k += (2 << d))	//pow(2, d + 1) = 2 << d
		{
			int t = dataset[k + (2 << (d - 1)) - 1];
			dataset[k + (2 << (d - 1)) - 1] = dataset[k + (2 << d) - 1];
			dataset[k + (2 << d) - 1] = t + dataset[k + (2 << d) - 1];
		}
	}
}
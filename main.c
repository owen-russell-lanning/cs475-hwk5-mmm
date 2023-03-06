#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "rtclock.h"
#include "mmm.h"
#include <math.h>

int main(int argc, char *argv[])
{
	double clockstart, clockend;

	if (argc < 3)
	{
		printf("%s\n", "Not enough args given");
		return 0;
	}

	// check mode to run in
	int seq_mode = 0;
	if (strcmp(argv[1], "S") == 0)
	{
		seq_mode = 1;
	}

	int threads = 1;
	int size = -1;

	// parse in cmdline args based on
	if (!seq_mode)
	{
		if (argc < 4)
		{
			printf("%s\n", "Not enough args given");
			return 0;
		}
		threads = atoi(argv[2]);
		size = atoi(argv[3]);

		printf("========\n");
		printf("mode: parallel\n");
		printf("thread count: %d\n", threads);
		printf("size: %d\n", size);
		printf("========\n");
	}
	else
	{

		size = atoi(argv[2]);
		printf("========\n");
		printf("mode: sequential\n");
		printf("thread count: 1\n");
		printf("size: %d\n", size);
		printf("========\n");
	}

	// check vars
	if (size <= 0 || threads <= 0)
	{
		printf("%s\n", "Invalid size or thread count. Threads and Size must be greater than zero.");
	}

	mmm_init(size, threads);

	// perform sequential average
	double totalRunTime = 0;
	for (int i = 0; i < 4; i++)
	{
		clockstart = rtclock(); // start clocking
		mmm_seq();
		clockend = rtclock(); // stop clocking

		if (i != 0)
		{
			totalRunTime += (clockend - clockstart);
		}
	}

	printf("Sequential Time (avg of 3 runs): %f sec\n", totalRunTime / 3);

	if (!seq_mode)
	{
		// calculate different squares or chunks of the matrix for each thread to calculate
		int rows = size / threads;
		int commonRowN = (int)(ceil(rows));
		int lastRowN = (int)(floor(rows));

		pthread_t *threadStructs = calloc(threads, sizeof(pthread_t));
		struct Mat_Params **paramLs = calloc(threads, sizeof(struct Mat_Params));

		// create threads and store into array
		for (int i = 0; i < threads; i++)
		{
			struct Mat_Params *params = malloc(sizeof(struct Mat_Params));
			paramLs[i] = params;
			params->start_row = i * commonRowN;
			params->end_row = params->start_row + commonRowN;
			if (i == threads - 1)
			{
				params->end_row = params->start_row + lastRowN;
			}
		}

		double totalParRunTime = 0;
		for (int i = 0; i < 4; i++)
		{
			clockstart = rtclock(); // start clocking

			// start thread execution
			for (int i = 0; i < threads; i++)
			{
				pthread_t pth;

				pthread_create(&pth, NULL, mmm_par, paramLs[i]);
				threadStructs[i] = pth;
			}

			// wait for all threads to finish
			for (int i = 0; i < threads; i++)
			{
				void *r;
				int result = pthread_join(threadStructs[i], r);
			}
			clockend = rtclock(); // stop clocking

			if (i != 0)
			{
				totalParRunTime += (clockend - clockstart);
			}
		}

		printf("Parallel Time (avg of 3 runs): %f sec\n", totalParRunTime / 3);
		printf("Speedup: %f\n", totalRunTime - totalParRunTime);

		free(paramLs);
	}

	// end: stuff I want to clock

	if (!seq_mode)
	{
		// verify the array
		printf("Verifying... ");
		double maxDif = mmm_verify();
		printf("largest error between parallel and sequential matrix: %f\n", maxDif);
	}

	// finished. now free up data
	mmm_freeup();
	return 0;
}

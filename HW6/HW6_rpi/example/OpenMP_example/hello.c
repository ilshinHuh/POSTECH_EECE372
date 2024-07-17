#include "omp.h"
#include <stdio.h>

int main() {

	omp_set_num_threads(4);
	
	#pragma omp parallel
	{
		int num_thread = omp_get_num_threads();
		int thread_ID = omp_get_thread_num();
		printf(" There are  %d number of threads.\n", num_thread);
		printf(" Hello world. [Thread num : %d]\n", thread_ID);
	}

	return 0;
}

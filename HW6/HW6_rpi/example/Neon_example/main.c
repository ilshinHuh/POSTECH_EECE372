#include <stdio.h> 
#include <stdlib.h> 
#include <arm_neon.h>
#include <time.h>

#include <string.h>           // needed for memset()
#include <math.h>             // needed for floating point routines
#include <sys/time.h>         // needed for gettimeofday()
#include <unistd.h>           // needed for pid_t type
#include <sys/ioctl.h>        // needed for ioctl()
#include <asm/unistd.h>       // needed for perf_event syscall
#include <linux/perf_event.h> // needed for perf_event
#include <omp.h>

#include <sys/time.h>
#include "arm_perf.h"

void func() {

	clock_t np0, np1, p0, p1;

	int *arr1 = malloc(sizeof(int32_t) * 4);
	int *arr2 = malloc(sizeof(int32_t) * 4);
  	int *arr3 = malloc(sizeof(int32_t) * 4);
  	int *ans = malloc(sizeof(int32_t) * 4);

  	int sum_for = 0;
  	int sum_neon = 0;

    time_t t;

    srand((unsigned) time(&t));

	for (int i = 0; i < 4; i++) {
		arr1[i] = rand()%7+1;
		arr2[i] = rand()%7+1;
	}

	int i = 4;

   ///////////////////////  Operation with C code starts  /////////////////
	np0 = clock();

	while (i) {
		ans[i-1] =  arr1[i-1] * arr2[i-1];
		sum_for += ans[i-1];
		i = i - 1;
	}

	np1 = clock();
	///////////////////////  Operation with C code ends  /////////////////
  
	i = 4;

	while (i) {
		printf("For loop result of each element : %d * %d => %d\n", arr1[i-1], arr2[i-1], ans[i-1]);
		i = i-1;
	}

	//////////////////  Change to your own code starts  //////////////////////
	p0 = clock(); // Operation starts after this line
	int32x4_t vec_1;
	int32x4_t vec_2;
	int32x4_t temp1 = vdupq_n_s32(0); // reset register to 0 value
  
	vec_1 = vld1q_s32(arr1);	// load arr1 values to vec_1 register
	vec_2 = vld1q_s32(arr2);	// load arr2 values to vec_2 register
	temp1 = vmulq_s32(vec_1, vec_2);	// multipicate vec_1 and vec_2

	vst1q_s32(arr3, temp1);	// store temp1 values to arr3 

	sum_neon = arr3[0] + arr3[1] + arr3[2] + arr3[3];
  
	p1 = clock();
	//////////////////  Chnage to your own code ends  //////////////////////////
  
	printf("lane0 is %d\n", vgetq_lane_s32(temp1, 3));	// show the value of lane3 in temp1
	printf("lane1 is %d\n", vgetq_lane_s32(temp1, 2));	// show the value of lane2 in temp1
	printf("lane2 is %d\n", vgetq_lane_s32(temp1, 1));	// show the value of lane1 in temp1
	printf("lane3 is %d\n", vgetq_lane_s32(temp1, 0));	// show the value of lane0 in temp1

	printf("For loop result: %d\n",sum_for);  
	printf("NEON result    : %d\n",sum_neon);
 
	printf("Execution time taken from FOR loop: %d\n", np1-np0);
	printf("Execution time taken from NEON    : %d\n", p1-p0);

	free(arr1);
	free(arr2);
	free(arr3);
	free(ans);
}


int main(int argc, char* argv[]) {

	func();

	return 0;
}


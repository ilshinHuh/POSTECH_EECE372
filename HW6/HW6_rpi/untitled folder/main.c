#include <arm_neon.h>
#include <asm/unistd.h>        // needed for perf_event syscall
#include <linux/perf_event.h>  // needed for perf_event
#include <math.h>              // needed for floating point routines
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>     // needed for memset()
#include <sys/ioctl.h>  // needed for ioctl()
#include <sys/time.h>   // needed for gettimeofday()
#include <sys/time.h>
#include <time.h>
#include <unistd.h>  // needed for pid_t type

#include "arm_perf.h"

void func() {
    clock_t np0, np1, p0, p1;

    int16_t *arr1 = malloc(sizeof(int16_t) * 8 * 8);
    int16_t *arr2 = malloc(sizeof(int16_t) * 8 * 8);
    int16_t *ans_neon = malloc(sizeof(int16_t) * 8 * 8);
    int16_t *ans_for = malloc(sizeof(int16_t) * 8 * 8);

    srand((unsigned)time(NULL));
    for (int i = 0; i < 8 * 8; i++) {
        arr1[i] = rand() % 15;
        arr2[i] = rand() % 15;
    }

    ///////////////////////  Matrix multiplication with for loop start /////////////////
    np0 = clock();

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                ans_for[8 * i + j] += arr1[8 * i + k] * arr2[8 * k + j];
            }
        }
    }

    np1 = clock();
    ///////////////////////  Matrix multiplication with for loop end  /////////////////

    ///////// Matrix multiplication with NEON start/////////
    p0 = clock();

    // get all the rows of arr1
    int16x8_t arr1_0 = vld1q_s16(arr1);
    int16x8_t arr1_1 = vld1q_s16(arr1 + 8);
    int16x8_t arr1_2 = vld1q_s16(arr1 + 16);
    int16x8_t arr1_3 = vld1q_s16(arr1 + 24);
    int16x8_t arr1_4 = vld1q_s16(arr1 + 32);
    int16x8_t arr1_5 = vld1q_s16(arr1 + 40);
    int16x8_t arr1_6 = vld1q_s16(arr1 + 48);
    int16x8_t arr1_7 = vld1q_s16(arr1 + 56);
    
    // get all the rows of arr2
    int16x8_t arr2_0 = vld1q_s16(arr2);
    int16x8_t arr2_1 = vld1q_s16(arr2 + 8);
    int16x8_t arr2_2 = vld1q_s16(arr2 + 16);
    int16x8_t arr2_3 = vld1q_s16(arr2 + 24);
    int16x8_t arr2_4 = vld1q_s16(arr2 + 32);
    int16x8_t arr2_5 = vld1q_s16(arr2 + 40);
    int16x8_t arr2_6 = vld1q_s16(arr2 + 48);
    int16x8_t arr2_7 = vld1q_s16(arr2 + 56);
    
    // set accumulator
    int16x8_t sum = vmovq_n_s16(0);
    
    // perform multiplication and accumulation for elements in row 1 of arr1 and all the rows of arr2
    sum = vmlaq_lane_s16(sum, arr2_0, vget_low_s16(arr1_0), 0);
    sum = vmlaq_lane_s16(sum, arr2_1, vget_low_s16(arr1_0), 1);
    sum = vmlaq_lane_s16(sum, arr2_2, vget_low_s16(arr1_0), 2);
    sum = vmlaq_lane_s16(sum, arr2_3, vget_low_s16(arr1_0), 3);
    sum = vmlaq_lane_s16(sum, arr2_4, vget_high_s16(arr1_0), 0);
    sum = vmlaq_lane_s16(sum, arr2_5, vget_high_s16(arr1_0), 1);
    sum = vmlaq_lane_s16(sum, arr2_6, vget_high_s16(arr1_0), 2);
    sum = vmlaq_lane_s16(sum, arr2_7, vget_high_s16(arr1_0), 3);
    vst1q_s16(ans_neon, sum);

    // reset accumulator to zero
    sum = vmovq_n_s16(0);
    
    // repeat for all rows of arr1
    sum = vmlaq_lane_s16(sum, arr2_0, vget_low_s16(arr1_1), 0);
    sum = vmlaq_lane_s16(sum, arr2_1, vget_low_s16(arr1_1), 1);
    sum = vmlaq_lane_s16(sum, arr2_2, vget_low_s16(arr1_1), 2);
    sum = vmlaq_lane_s16(sum, arr2_3, vget_low_s16(arr1_1), 3);
    sum = vmlaq_lane_s16(sum, arr2_4, vget_high_s16(arr1_1), 0);
    sum = vmlaq_lane_s16(sum, arr2_5, vget_high_s16(arr1_1), 1);
    sum = vmlaq_lane_s16(sum, arr2_6, vget_high_s16(arr1_1), 2);
    sum = vmlaq_lane_s16(sum, arr2_7, vget_high_s16(arr1_1), 3);
    vst1q_s16(ans_neon + 8, sum);
    
    sum = vmovq_n_s16(0);
    
    sum = vmlaq_lane_s16(sum, arr2_0, vget_low_s16(arr1_2), 0);
    sum = vmlaq_lane_s16(sum, arr2_1, vget_low_s16(arr1_2), 1);
    sum = vmlaq_lane_s16(sum, arr2_2, vget_low_s16(arr1_2), 2);
    sum = vmlaq_lane_s16(sum, arr2_3, vget_low_s16(arr1_2), 3);
    sum = vmlaq_lane_s16(sum, arr2_4, vget_high_s16(arr1_2), 0);
    sum = vmlaq_lane_s16(sum, arr2_5, vget_high_s16(arr1_2), 1);
    sum = vmlaq_lane_s16(sum, arr2_6, vget_high_s16(arr1_2), 2);
    sum = vmlaq_lane_s16(sum, arr2_7, vget_high_s16(arr1_2), 3);
    vst1q_s16(ans_neon + 16, sum);
    
    sum = vmovq_n_s16(0);
    
    sum = vmlaq_lane_s16(sum, arr2_0, vget_low_s16(arr1_3), 0);
    sum = vmlaq_lane_s16(sum, arr2_1, vget_low_s16(arr1_3), 1);
    sum = vmlaq_lane_s16(sum, arr2_2, vget_low_s16(arr1_3), 2);
    sum = vmlaq_lane_s16(sum, arr2_3, vget_low_s16(arr1_3), 3);
    sum = vmlaq_lane_s16(sum, arr2_4, vget_high_s16(arr1_3), 0);
    sum = vmlaq_lane_s16(sum, arr2_5, vget_high_s16(arr1_3), 1);
    sum = vmlaq_lane_s16(sum, arr2_6, vget_high_s16(arr1_3), 2);
    sum = vmlaq_lane_s16(sum, arr2_7, vget_high_s16(arr1_3), 3);
    vst1q_s16(ans_neon + 24, sum);
    
    sum = vmovq_n_s16(0);
    
    sum = vmlaq_lane_s16(sum, arr2_0, vget_low_s16(arr1_4), 0);
    sum = vmlaq_lane_s16(sum, arr2_1, vget_low_s16(arr1_4), 1);
    sum = vmlaq_lane_s16(sum, arr2_2, vget_low_s16(arr1_4), 2);
    sum = vmlaq_lane_s16(sum, arr2_3, vget_low_s16(arr1_4), 3);
    sum = vmlaq_lane_s16(sum, arr2_4, vget_high_s16(arr1_4), 0);
    sum = vmlaq_lane_s16(sum, arr2_5, vget_high_s16(arr1_4), 1);
    sum = vmlaq_lane_s16(sum, arr2_6, vget_high_s16(arr1_4), 2);
    sum = vmlaq_lane_s16(sum, arr2_7, vget_high_s16(arr1_4), 3);
    vst1q_s16(ans_neon + 32, sum);
    
    sum = vmovq_n_s16(0);
    
    sum = vmlaq_lane_s16(sum, arr2_0, vget_low_s16(arr1_5), 0);
    sum = vmlaq_lane_s16(sum, arr2_1, vget_low_s16(arr1_5), 1);
    sum = vmlaq_lane_s16(sum, arr2_2, vget_low_s16(arr1_5), 2);
    sum = vmlaq_lane_s16(sum, arr2_3, vget_low_s16(arr1_5), 3);
    sum = vmlaq_lane_s16(sum, arr2_4, vget_high_s16(arr1_5), 0);
    sum = vmlaq_lane_s16(sum, arr2_5, vget_high_s16(arr1_5), 1);
    sum = vmlaq_lane_s16(sum, arr2_6, vget_high_s16(arr1_5), 2);
    sum = vmlaq_lane_s16(sum, arr2_7, vget_high_s16(arr1_5), 3);
    vst1q_s16(ans_neon + 40, sum);
    
    sum = vmovq_n_s16(0);
    
    sum = vmlaq_lane_s16(sum, arr2_0, vget_low_s16(arr1_6), 0);
    sum = vmlaq_lane_s16(sum, arr2_1, vget_low_s16(arr1_6), 1);
    sum = vmlaq_lane_s16(sum, arr2_2, vget_low_s16(arr1_6), 2);
    sum = vmlaq_lane_s16(sum, arr2_3, vget_low_s16(arr1_6), 3);
    sum = vmlaq_lane_s16(sum, arr2_4, vget_high_s16(arr1_6), 0);
    sum = vmlaq_lane_s16(sum, arr2_5, vget_high_s16(arr1_6), 1);
    sum = vmlaq_lane_s16(sum, arr2_6, vget_high_s16(arr1_6), 2);
    sum = vmlaq_lane_s16(sum, arr2_7, vget_high_s16(arr1_6), 3);
    vst1q_s16(ans_neon + 48, sum);
    
    sum = vmovq_n_s16(0);
    
    sum = vmlaq_lane_s16(sum, arr2_0, vget_low_s16(arr1_7), 0);
    sum = vmlaq_lane_s16(sum, arr2_1, vget_low_s16(arr1_7), 1);
    sum = vmlaq_lane_s16(sum, arr2_2, vget_low_s16(arr1_7), 2);
    sum = vmlaq_lane_s16(sum, arr2_3, vget_low_s16(arr1_7), 3);
    sum = vmlaq_lane_s16(sum, arr2_4, vget_high_s16(arr1_7), 0);
    sum = vmlaq_lane_s16(sum, arr2_5, vget_high_s16(arr1_7), 1);
    sum = vmlaq_lane_s16(sum, arr2_6, vget_high_s16(arr1_7), 2);
    sum = vmlaq_lane_s16(sum, arr2_7, vget_high_s16(arr1_7), 3);
    vst1q_s16(ans_neon + 56, sum);
    
    p1 = clock();
    ///////// Matrix multiplication with NEON end///////////

    int check = 0;
    for (int i = 0; i < 8 * 8; i++) {
        if (ans_neon[i] != ans_for[i]) {
            check += 1;
        }
    }
    if (check == 0) {
        printf("PASS\n");
    }
    else {
        printf("FAIL\n");
    }

    printf("Execution time (for) : %7.3lf[us]\n", ((double)np1 - np0) / ((double)CLOCKS_PER_SEC / 1000000));
    printf("Execution time (NEON): %7.3lf[us]\n", ((double)p1 - p0) / ((double)CLOCKS_PER_SEC / 1000000));

    free(arr1);
    free(arr2);
    free(ans_for);
    free(ans_neon);
    return;
}
int main(int argc, char *argv[]) {
    func();

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// user defined functions
void mergesort_C(int *array, int left, int right); // merge sort in C
void merge_C(int *array, int left, int middle, int right); // merge in C
void mergesort_ASM(int* array, int N); // merge sort in Assembly

void number_producer(int* arr, int N); // produce random N numbers
void swap(int* a, int* b); // array element swapper
void printer(int* arr, int size); // array printer
void copy_array(int* original_array, int* copied_array, int N); // array copy function

int main(int argc, char* argv[]){
    srand(time(NULL));

    int N = atoi(argv[1]);
    
    if(N < 1) // exception handling
        return 1;
        
    int* before_array = (int*) malloc(sizeof(int) * N); // dynamically allocate array L
    int* after_C_array = (int*) malloc(sizeof(int) * N); // dynamically allocate array for result(C)
    int* after_ASM_array = (int*) malloc(sizeof(int) * N); // dynamically allocate array for result(ASM)

    number_producer(before_array, N);
    copy_array(before_array, after_C_array, N);
    copy_array(before_array, after_ASM_array, N);

    // print data before sorting
    if(N <= 20){
        printf("Before sort     : [ ");
        printer(before_array, N);
        printf("]\n");
    }

    struct timespec start_C, end_C, start_ASM, end_ASM; // set clock variablesa

    // measuring time for C
    clock_gettime(CLOCK_MONOTONIC, &start_C);
    mergesort_C(after_C_array, 0, N - 1);
    clock_gettime(CLOCK_MONOTONIC, &end_C);

    // measuring time for Assembly
    clock_gettime(CLOCK_MONOTONIC, &start_ASM);
    mergesort_ASM(after_ASM_array, N);
    clock_gettime(CLOCK_MONOTONIC, &end_ASM);

    // print data after sorting
    if(N <= 20){
        printf("After sort   (C): [ ");
        printer(after_C_array, N);
        printf("]\n");
        printf("After sort (ASM): [ ");
        printer(after_ASM_array, N);
        printf("]\n");
    }

    // print run time
    printf("Execution Time   (C): %.6lf[s]\n", (double) (end_C.tv_sec - start_C.tv_sec) + (double) (end_C.tv_nsec - start_C.tv_nsec) / 1e9);
    printf("Execution Time (ASM): %.6lf[s]\n", (double) (end_ASM.tv_sec - start_ASM.tv_sec) + (double) (end_ASM.tv_nsec - start_ASM.tv_nsec) / 1e9);

    return 0;
}

void mergesort_C(int *array, int left, int right) {
    if(right > left){ // continue when subarray has more than one element
        int middle = (left + right) / 2; // find the middle point
        mergesort_C(array, left, middle); // call mergesort for left half
        mergesort_C(array, middle + 1, right); // call mergesort for right half

        merge_C(array, left, middle, right); // merge two halves
    }

    return;
}

void merge_C(int *array, int left, int middle, int right) {
    int i, j, k;

    int temp_array_length = right - left + 1; // calculate the length of temporary array
    int temp_array[temp_array_length]; // allocate temporary array

    i = left; // start index for the first subarray
    j = middle + 1; // start index for the second subarray
    k = 0; // start index for the temporary array

    while (i <= middle && j <= right) { // copy datas to temp array
        if (array[i] <= array[j]) {
            temp_array[k] = array[i];
            k++;
            i++;
        } else {
            temp_array[k] = array[j];
            k++;
            j++;
        }
    }

    // copy remaining elements from the left subarray
    while (i <= middle) {
        temp_array[k] = array[i];
        k++;
        i++;
    }

    // copy remaining elements from the right subarray
    while (j <= right) {
        temp_array[k] = array[j];
        k++;
        j++;
    }

    // reconstruct the original array
    for (i = 0; i < temp_array_length; i++) {
        array[left + i] = temp_array[i];
    }

    return;
}

void mergesort_ASM(int *array, int N) {
    asm (
        "mov r0, #0\n\t" // initilaize left = 0
        "ldr r1, %[N]\n\t" // r1 = N
        "sub r1, r1, #1\n\t" // r1 = right = N - 1
        "bl merge_sort\n\t" // call merge_sort
        "b end\n\t" // end mergesort
        
    //----------------------------------------
    // merge starts here
    "merge:\n\t"
        "push {lr}\n\t" // save return address
        "mov r3, r0\n\t" // i = left
        "add r4, r1, #1\n\t" // j = middle + 1
        "mov r5, #0\n\t" // k = 0
        
        "sub r9, r2, r0\n\t" // r9 = right - left
        "add r9, r9, #1\n\t" // r9 = temp_array_length
        "lsl r9, r9, #2\n\t" // r9 = temp_array_length * sizeof(int)
        "sub sp, sp, r9\n\t" // allocate array

        "lsr r9, r9, #2\n\t" // r9 = temp_array_length
        
        // copy datas to temp array
        // while (i <= middle && j <= right)
    "loop1:\n\t"
        "cmp r3, r1\n\t" // compare i and middle
        "bgt copy_remnant_left\n\t" // end loop1
        "cmp r4, r2\n\t" // compare j and right
        "bgt copy_remnant_left\n\t" // end loop1
        
        // if(array[i] <= array[j])
        "ldr r6, [%[array], r3, LSL #2]\n\t" // r6 = array[i]
        "ldr r8, [%[array], r4, LSL #2]\n\t" // r8 = array[j]
        
        "cmp r6, r8\n\t" // compare array[i] and array[j]
        "bgt if2\n\t" // else
        // if (array[i] <= array[j])
    "if1:\n\t"
        "str r6, [sp, r5, LSL #2]\n\t" // temp_array[k] = array[i]
        "add r5, #1\n\t" // k++
        "add r3, #1\n\t" // i++
        "b loop1\n\t"
        // else
    "if2:\n\t"
        "str r8, [sp, r5, LSL #2]\n\t" // temp_array[k] = array[j]
        "add r5, #1\n\t" // k++
        "add r4, #1\n\t" // j++
        "b loop1\n\t"
            
        // copy remaining elements from left
        // while(i <= middle)
    "copy_remnant_left:\n\t"
        "cmp r3, r1\n\t"
        "bgt copy_remnant_right\n\t"
        "ldr r6, [%[array], r3, LSL #2]\n\t" // r6 = array[i]
        "str r6, [sp, r5, LSL #2]\n\t" // temp_array[k] = array[i]
        "add r5, #1\n\t" // k++
        "add r3, #1\n\t" // i++
        "b copy_remnant_left\n\t"
        
        // copy remaining elements from right
        // while (j <= right)
    "copy_remnant_right:\n\t"
        "cmp r4, r2\n\t"
        "bgt copy_end\n\t"
        "ldr r6, [%[array], r4, LSL #2]\n\t" // r6 = array[j]
        "str r6, [sp, r5, LSL #2]\n\t" // temp_array[k] = array[j]
        "add r5, #1\n\t" // k++
        "add r4, #1\n\t" // j++
        "b copy_remnant_right\n\t"
    
    "copy_end:\n\t"
        "mov r5, #0\n\t" // i = 0
        // reconstruct original array
    "reconstruct:\n\t"
        "cmp r5, r9\n\t" // compare i and temp_array_length
        "bge merge_end\n\t" // return merge if i >= temp_array_length
        "ldr r6, [sp, r5, LSL #2]\n\t" // r6 = temp_array[i]
        "add r4, r0, r5\n\t" // left + i
        "str r6, [%[array], r4, LSL #2]\n\t" // array[left + i] = temp_array[i]
        "add r5, r5, #1\n\t" // i++
        "b reconstruct\n\t"
        
    "merge_end:\n\t"
        "lsl r9, r9, #2\n\t" // r9 = temp_array_length * sizeof(int)
        "add sp, sp, r9\n\t" // deallocate array
        "pop {pc}\n\t" // return
    // merge ends here
    //----------------------------------------
    // merge_sort starts here
    "merge_sort:\n\t"
    // r0 = left
    // r1 = right
        "push {r0, r1, lr}\n\t" // save previous variables
        
        "cmp r0, r1\n\t" // compare left and right
        "bge merge_sort_end\n\t" // end merge_sort if array has less than 2 elements
        
        "sub r2, r1, r0\n\t" // r2 = right - left
        "lsr r2, r2, #1\n\t" // r2 = (right - left) / 2
        "add r2, r0, r2\n\t" // r2 = middle = left + (right - left) / 2
        
        "push {r1}\n\t" // right saved, r0 = left r2 = middle
        "mov r1, r2\n\t" // r0 = left, r1 = middle
        "bl merge_sort\n\t" // recursive call on left subarray
        "pop {r2}\n\t" // r2 = right
        
        "push {r0}\n\t" // left saved
        "mov r0, r1\n\t" // r0 = middle
        "add r0, r0, #1\n\t" // r0 = middle + 1
        "mov r1, r2\n\t" // r1 = right
        "bl merge_sort\n\t" // recursive call on right subarray
        
        "mov r2, r1\n\t" // r2 = right
        "mov r1, r0\n\t" // r1 = middle + 1
        "sub r1, r1, #1\n\t" // r1 = middle
        "pop {r0}\n\t" // r0 = left
        
        // r0 = left, r1 = middle, r2 = right
        "bl merge\n\t" // call merge
        
    "merge_sort_end:\n\t"
        "pop {r0, r1, pc}\n\t" // restore original variables and return
    // merge_sort ends here
    //----------------------------------------
    "end:\n\t"

        : // no output operands
        : [array] "r" (array), [N] "m" (N) // input operands
        : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r8", "r9", "pc", "lr" // clobbered registers
    );

    return;
}
    

// produce random N numbers
void number_producer(int* arr, int N){
    srand(time(NULL));
    
    int i;
    for (i = 0; i < N; i++) {
        arr[i] = (i + 1);
    }

    // Knuth shuffle
    for (i = 0; i < N - 1; i++) {
        int j = i + rand() % (N - i);
        swap(&arr[i], &arr[j]);
    }

    return;
}

// number swapper
void swap(int* a, int* b){
    int temp = *b;
    *b = *a;
    *a = temp;

    return;
}

// print every elemtent of an array
void printer(int* arr, int size){
    int i;
    for(i = 0; i < size; i++){
        printf("%d ", arr[i]);// print every element of an array
    }

    return;
}

// copy array to another array
void copy_array(int* original_array, int* copied_array, int N){
    int i;

    for(i = 0; i < N; i++){
        copied_array[i] = original_array[i];
    }

    return;
}

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// user defined functions
void insertion_C(int* array, int N); // insertion sort in C
void insertion_ASM(int* array, int N); // insertion sort in Assembly

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
    insertion_C(after_C_array, N);
    clock_gettime(CLOCK_MONOTONIC, &end_C);

    // measuring time for Assembly
    clock_gettime(CLOCK_MONOTONIC, &start_ASM);
    insertion_ASM(after_ASM_array, N);
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

void insertion_C(int* array, int N) {
    int i, j, key;
    for(int i = 1; i < N; i++){
        key = array[i]; // take an unsorted element
        j = i - 1;

        while(array[j] > key && j >= 0){ // loop over sorted elements
            array[j + 1] = array[j]; // move elements to the right
            j--;
        }
        array[j + 1] = key; // insert element into the correct position
    }

    return;
}

void insertion_ASM(int* array, int N) {
    asm(
        "ldr r0, %[N]\n\t" // r0 = N
        "mov r1, #1\n\t" // r1 = i = 1

    "outer_loop:\n\t"
        "cmp r1, r0\n\t" // compare i with N
        "bge end_outer_loop\n\t" // exit loop if i >= N

        "ldr r2, [%[array], r1, LSL #2]\n\t" // r2 = array[i]

        "sub r3, r1, #1\n\t" // r3 = i - 1

    "inner_loop:\n\t"
        "ldr r4, [%[array], r3, LSL #2]\n\t" // r4 = array[j]
        "cmp r4, r2\n\t" // compare array[j] with key
        "ble end_inner_loop\n\t" // exit loop if array[j] <= key

        "cmp r3, #0\n\t" // compare j with 0
        "blt end_inner_loop\n\t" // exit loop if j < 0

        "add r5, r3, #1\n\t" // r5 = j + 1
        "str r4, [%[array], r5, LSL #2]\n\t" // array[j+1] = array[j]

        "sub r3, r3, #1\n\t" // decrement j
        "b inner_loop\n\t" // continue inner loop

    "end_inner_loop:\n\t"
        "add r5, r3, #1\n\t" // r5 = j + 1
        "str r2, [%[array], r5, LSL #2]\n\t" // array[j + 1] = key

        "add r1, r1, #1\n\t" // increment i by 1
        "b outer_loop\n\t" // continue outer loop

    "end_outer_loop:\n\t"

        : // no output operands
        : [array] "r" (array), [N] "m" (N) // input operands
        : "memory", "r0", "r1", "r2", "r3", "r4", "r5" // clobbered registers
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

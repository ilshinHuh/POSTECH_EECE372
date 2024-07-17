#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int digit_nums(int n);// get number of digits of an integer
void prod_MultofThree(int* arr, int n);// procude random multiples of three
void swap(int* a, int* b);// number swap function
void bubble_sort(int* arr, int n, int order);// buuble sort
void selection_sort(int* arr, int n, int order);// selection sort
void printer(int* arr, int size);// array printer function

int main(int argc, char* argv[]){

    // exception handling #1
    if(argc != 4|| (strcmp(argv[2], "0") != 0 && strcmp(argv[2], "1") != 0) || \
    (strcmp(argv[3], "0") != 0 && strcmp(argv[3], "1") != 0)) {
        printf("Error: wrong input type.\n");
        return 1;
    }

    int n = atoi(argv[1]); // get n

    // exception handling #2
    if(digit_nums(n) != strlen(argv[1]) || n < 1 || n > 300000){
        printf("Error: wrong input type.\n");
        return 1;
    } else if(n < 3){
        printf("No multiples of three in range.\n");
        return 1;
    }

    int order = atoi(argv[2]); // get order of sorting
    int method = atoi(argv[3]); // get method of sorting

    int* arr = (int*) malloc(sizeof(int) * (n / 3));// dynamically allocate array
    int* arr_copy = (int*) malloc(sizeof(int) * (n / 3));// for saving original array

    prod_MultofThree(arr, n);// put randomly generated numbers in array
    
    int i;
    for(i = 0; i < n / 3; i++){// copy original array
        arr_copy[i] = arr[i];
    }

    clock_t start, end;
    switch(method){// sort by input method
        case 0:
            start = clock();
            bubble_sort(arr, n, order);
            end = clock();
            printf("bubble sort: %lf sec\n", (double) (end - start) / CLOCKS_PER_SEC);
            break;
        case 1:
            start = clock();
            selection_sort(arr, n, order);
            end = clock();
            printf("selection sort: %lf sec\n", (double) (end - start) / CLOCKS_PER_SEC);
            break;
    }

    if(n <= 30){// print sorted and unsorted array if n <= 30
        printf("before sort: ");
        printer(arr_copy, n / 3);
        printf("after sort: ");
        printer(arr, n / 3);
    }

    free(arr);
    free(arr_copy);// free dynamically allocated memory

    return 0;
}

// get number of digits of an integer recursively
int digit_nums(int n){
    if(n == 0){
        return 0;
    }
    while(n != 0){
        return 1 + digit_nums(n / 10);
    }
    return 0;
}

// produce random multiples of three using srand
void prod_MultofThree(int* arr, int n){
    srand(time(NULL));
    
    int i;
    for(i = 0; i < n / 3; i++){
        int gen_num = 3 * (rand() % (n / 3) + 1);// generate random number

        int j = 0;
        while(j < i){
            if(arr[j] == gen_num){// check same number
                gen_num = 3 * (rand() % (n / 3) + 1);
                j = -1;// restart loop from beginning
            }
            j++;
        }
        arr[i] = gen_num;// save generated random number in input array
    }
}

// number swapper
void swap(int* a, int* b){
    int temp = *b;
    *b = *a;
    *a = temp;
}

// bubble sort
void bubble_sort(int* arr, int n, int order){
    int i, j, swapped = 1;

    for(i = 0; i < n / 3 - 1 && swapped == 1; i++){
        swapped = 0;
        for(j = 0; j < n / 3 - i - 1; j++){
            if(order == 0){
                if(arr[j] > arr[j + 1]){
                    swap(&arr[j], &arr[j + 1]);
                    swapped = 1;
                }
            } else if(order == 1){
                if(arr[j] < arr[j + 1]){
                    swap(&arr[j], &arr[j + 1]);
                    swapped = 1;
                }
            }
        }
    }
}

// selection sort
void selection_sort(int* arr, int n, int order){
    int i, j, minmax_idx;

    for(i = 0; i < n / 3 - 1; i++){
        minmax_idx = i;

        for(j = i + 1; j < n / 3; j++){
            if(order == 0){
                if(arr[j] < arr[minmax_idx]){
                    minmax_idx = j;
                }
            } else if(order == 1){
                if(arr[j] > arr[minmax_idx]){
                    minmax_idx = j;
                }
            }
        }

        swap(&arr[i], &arr[minmax_idx]);
    }
}

// print every elemtent of an array
void printer(int* arr, int size){
    int i;
    for(i = 0; i < size - 1; i++){
        printf("%d ", arr[i]);// print every element of an array
    }
    printf("%d\n", arr[size - 1]);
}
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// node structure of pq linked list
typedef struct Node {
    int key;
    int data;
    struct Node* next;
} Node;

// node structure of pq array
typedef struct {
    int key;
    int data;
} Node_arr;

// pq array structure
typedef struct {
    Node_arr* arr;
    int head, tail;
} Queue_array;

// pq linked list structure
typedef struct {
    Node *front, *rear;
} Queue_linkedList;

void prod_MultofK(int* arr, int k, int n); // produce random n multiples of k using srand in array
void swap(int* a, int* b); // number swapper
void printer(int* arr, int size); // array printer

Queue_array* create_pqarray(int size); // create priority queue array
void insert_arr(Queue_array* q, int value); // insert element in pq array
int deleteMin_arr(Queue_array* q); // delete min element of pq array

Node* create_node(int value); // create new node of linked list
Queue_linkedList* create_pqlinkedList(); // create pq linked list
void insert_linkedList(Queue_linkedList* q, int value); // insert node in pq linked list
int deleteMin_linkedList(Queue_linkedList* q); // delete min node of linked list

int main(int argc, char* argv[]){
    // exception handling #1
    if(argc != 3) {
        printf("Error: Invalid input entered.\n");
        return 1;
    }

    int k = atoi(argv[1]); // get k
    int n = atoi(argv[2]); // get n

    // exception handling #2
    if(k < 1 || k > 10 || n < 1 || n > 300000) {
        printf("Error: Invalid input entered.\n");
        return 1;
    }

    int* arrL = (int*) malloc(sizeof(int) * n); // dynamically allocate array L
    int* arr_result_arrL = (int*) malloc(sizeof(int) * n); // dynamically allocate array for result(array)
    int* LL_result_arrL = (int*) malloc(sizeof(int) * n); // dynamically allocate array for result(Linked-list)

    prod_MultofK(arrL, k, n); // put randomly generated numbers in arrL

    // create pq array
    Queue_array* pq_array = create_pqarray(n);

    // dyanamically allocate pq Linked list
    Queue_linkedList* pq_linkedList = create_pqlinkedList();

    struct timespec start1, end1, start2, end2; // set clock variables

    int i;
    // array sort & delete
    clock_gettime(CLOCK_MONOTONIC, &start1);
    for(i = 0; i < n; i++){
        insert_arr(pq_array, arrL[i]); // insert one element at a time wioth priority in array
    }
    for(i = 0; i < n; i++){
        arr_result_arrL[i] = deleteMin_arr(pq_array); // delete and get an element at a time in array
    }
    clock_gettime(CLOCK_MONOTONIC, &end1);

    // Linked-list sort & delete
    clock_gettime(CLOCK_MONOTONIC, &start2);
    for(i = 0; i < n; i++){
        insert_linkedList(pq_linkedList, arrL[i]); // insert one element at a time wioth priority in Linked-list
    }
    for(i = 0; i < n; i++){
        LL_result_arrL[i] = deleteMin_linkedList(pq_linkedList); // delete and get an element at a time in Linked-list
    }
    clock_gettime(CLOCK_MONOTONIC, &end2);

    if(n <= 10){ // print unsorted and sorted array
        printf("before sort: ");
        printer(arrL, n);
        printf("after sort: ");
        printer(arr_result_arrL, n);
        //printf("after sort: ");
        //printer(LL_result_arrL, n);
    }
    printf("[Execution Time]\nArray: %.6lf sec\nLinked-list: %.6lf sec\n", (double) (end1.tv_sec - start1.tv_sec) + (double) (end1.tv_nsec - start1.tv_nsec) / 1e9, (double) (end2.tv_sec - start2.tv_sec) + (double) (end2.tv_nsec - start2.tv_nsec) / 1e9); // print execution time for each case

    // free allocated memories
    free(arrL);    
    free(LL_result_arrL);
    free(arr_result_arrL);
    free(pq_array->arr);
    free(pq_array);
    free(pq_linkedList);

    return 0;
}

// produce random n multiples of k using srand in array
void prod_MultofK(int* arr, int k, int n){
    srand(time(NULL));
    
    int i;
    for (i = 0; i < n; i++) {
        arr[i] = (i + 1) * k;
    }

    // Knuth shuffle
    for (i = 0; i < n - 1; i++) {
        int j = i + rand() % (n - i);
        swap(&arr[i], &arr[j]);
    }
}

// number swapper
void swap(int* a, int* b){
    int temp = *b;
    *b = *a;
    *a = temp;
}

// print every elemtent of an array
void printer(int* arr, int size){
    int i;
    for(i = 0; i < size - 1; i++){
        printf("%d ", arr[i]);// print every element of an array
    }
    printf("%d\n", arr[size - 1]);
}

// create qriority queue array
Queue_array* create_pqarray(int size){
    Queue_array* q = (Queue_array*)malloc(sizeof(Queue_array)); // dynamically allocate memory for array
    q->arr = (Node_arr*)malloc(sizeof(Node_arr) * size);
    q->head = 0;
    q->tail = -1; // to indicate no element
    return q;
}

// insert element in pq array
void insert_arr(Queue_array* q, int value){
    q->tail = q->tail + 1; // increment tail pointing integer by 1

    int i = q->tail;
    while(i > q->head && value < (q->arr[i - 1]).data) {
        q->arr[i] = q->arr[i - 1]; // move all element bigger than new key to the right
        i--;
    }
    q->arr[i].key = value; // insert value in the right place
    q->arr[i].data = value;
}

// delete min element of pq array
int deleteMin_arr(Queue_array* q){
    int temp = q->arr[q->head].data;
    
    int i;
    for(i = q->head; i < q->tail; i++){
        q->arr[i] = q->arr[i + 1]; // move every element to the left
    }

    q->arr[q->tail].data = -1; // delete data in the place of current tail
    q->arr[q->tail].key = -1; // delete key in the place of current tail
    q->tail = q->tail - 1; // decrement tail pointing integer by 1
    return temp; // return min key
}

// create new node for Linked-list
Node* create_node(int value) {
    Node* new_node = (Node*)malloc(sizeof(Node)); // dynamically allocate memory for new node
    new_node->key = value;
    new_node->data = value;
    new_node->next = NULL;
    return new_node;
}

// create qriority queue linked list
Queue_linkedList* create_pqlinkedList(){
    Queue_linkedList* q = (Queue_linkedList*)malloc(sizeof(Queue_linkedList)); // dynamically allocate memory for Linked-list queue
    q->front = NULL; // set front and rear node to NULL
    q->rear = NULL;
    return q;
}

// insert element in pq linked list
void insert_linkedList(Queue_linkedList* q, int value){
    Node* new_node = create_node(value); // create new node

    // If the linked list is empty or the new node has higher priority than the front node
    if (q->front == NULL || value < q->front->data) {
        new_node->next = q->front; // Set new node as the new front
        q->front = new_node;
        if (q->rear == NULL) { // If the linked list was empty
            q->rear = new_node; // Set new node as rear node
        }
        return;
    }

    Node* curr = q->front;
    while(curr->next != NULL && value > curr->next->data){ // find location for new node by traversing through Linked-list
        curr = curr->next;
    }
    new_node->next = curr->next;
    curr->next = new_node;
    if(new_node->next == NULL){
        q->rear = new_node;
    }
}

// delete min element of pq linked list
int deleteMin_linkedList(Queue_linkedList* q){ // delete(free memory) front node and return the value of it
    Node* curr = q->front;
    int temp = q->front->data;
    q->front = q->front->next;
    free(curr);
    return temp;
}
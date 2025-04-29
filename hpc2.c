#include <stdio.h>
#include<stdlib.h>
#include<omp.h>
#include<time.h>

#define MAX  100000

void generateRandomArray(int * arr, int n){
    for(int i = 0; i < n; i++){
        arr[i] = rand() % 10000;
    }
}

void bubbleSortSeq(int * arr, int n){
    for(int i = 0; i < n - 1; i++){
        for(int j = 0; j < n - i -1; j++){
            if(arr[j] > arr[j+1]){
                int temp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = temp;
            }
        }
    }
}

void bubbleSortParallel(int * arr, int n){
    for(int i = 0; i < n; i++){
        int start = i % 2;
        #pragma omp parallel for
        for(int j = start; j < n -1; j+=2){
            if(arr[j]> arr[j+1]){
                int temp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = temp;
            }
        }
    }
}

void printArray(int * arr, int n){
    for(int i = 0; i < n && i < 20; i++){
        printf("%5d ", arr[i]);
    }
    if(n > 20){
        printf("...");
        
    }
    printf("\n");
}


int main(){
    int n;
    printf("Enter number   of elements: ");
    scanf("%d", &n);
    int * arr1 = malloc(n* sizeof(int));
    int * arr2 = malloc(n* sizeof(int));
    srand(time(NULL));

    generateRandomArray(arr1, n);
    for(int i = 0 ; i < n; i++) arr2[i] = arr1[i];


    printf("\n Original Array:\n");
    printArray(arr1, n);
    
    double start, end;

    start = omp_get_wtime();
    bubbleSortParallel(arr2, n);
    end = omp_get_wtime();

    double parallelTime = end - start;


    start = omp_get_wtime();
    bubbleSortSeq(arr1, n);
    end = omp_get_wtime();
    double sequentialTime = end - start;

    printf("\n Sorted (sequential): \n");
    printArray(arr1, n);
    printf("TIme (Sequential): %.6f s\n", sequentialTime);


    printf("\n Sorted (parallel): \n");
    printArray(arr2, n);
    printf("Time (parallel) %.6f s \n", parallelTime);

    printf("\n %-25s %.2f speedup", "Speedup par/seq:", sequentialTime/parallelTime);
    free(arr1);
    free(arr2);
    return 0;

}
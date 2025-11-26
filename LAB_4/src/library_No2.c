#include "../include/library.h"

#ifdef _MSC_VER
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

void fillPrimeNumbers(int* primesNumbers, int lastPos){
    for(int i = 0; i <= lastPos; ++i){
        primesNumbers[i] = i;
    }
    for(int p = 2; p <= lastPos; ++p){
        if(primesNumbers[p] != 0){
            
            for(long long j = (long long)p * p; j <= lastPos; j += p){
                if(j <= lastPos){
                    primesNumbers[(int)j] = 0;
                }
            }
        }
    }
}

EXPORT int f_prime_count(int a, int b){
    int primeNmbers[b+1];
    fillPrimeNumbers(primeNmbers, b);

    int answer = 0;
    for(int i = a; i <= b; ++i){
        if(primeNmbers[i] != 0 && primeNmbers[i] >= 2) ++answer;
    }
    return answer;
}

size_t partition(int *arr, size_t left, size_t right) {
    int mid = arr[(left + right) / 2];
    int i = left, j = right - 1;
    int temp;

    do {
        while (i < right && arr[i] < mid) {
            ++i;
        }

        while (j >= left && arr[j] > mid) {
            --j;
        }

        if (i <= j) {
            temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
            ++i;
            --j;
        }
    } while (i <= j);
    return j;
}

void quicksort(int *arr, size_t left, size_t right) {
    if (right - left < 2) {
        return;
    }
    size_t p = partition(arr, left, right);
    quicksort(arr, left, p + 1);
    quicksort(arr, p + 1, right);
}

EXPORT int *f_sort(int *array, size_t n){
    quicksort(array, 0, n);
    return array;
}
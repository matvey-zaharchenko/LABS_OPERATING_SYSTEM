#include "../include/library.h"

#ifdef _MSC_VER
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

bool isPrime(int number){
    if(number < 2) return false;
    for(int i = 2; i < (int)(sqrt(number) + 1); ++i){
        if(number % i == 0) return false;
    }
    return true;
}

EXPORT int f_prime_count(int a, int b){
    int answer = 0;
    for(int i = a; i <= b; ++i){
        if(isPrime(i) && i >= 2) ++answer;
    }
    return answer;
}

void swap(int *a, int *b){
    int temp = *a;
    *a = *b;
    *b = temp;
}

EXPORT int *f_sort(int *array, size_t n){
    for(int i = 0; i < n - 1; ++i){
        bool change = false;
        for(int j = 0; j < n - i - 1; ++j){
            if(array[j] > array[j+1]){
                change = true;
                swap(&array[j], &array[j+1]);              
            }
        }
        if(!change) break;
    }
    return array;
}
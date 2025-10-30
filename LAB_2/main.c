#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <sys/time.h>
#include <stdbool.h>

static int num_threads = 1;
static sem_t thread_limiter;

typedef struct {
    int *arr;
    size_t left;
    size_t right;
} ThreadArgs;

static size_t partition(int *arr, size_t left, size_t right) {
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

static void quicksort_seq(int *arr, size_t left, size_t right) {
    if (right - left < 2) {
        return;
    }
    size_t p = partition(arr, left, right);
    quicksort_seq(arr, left, p + 1);
    quicksort_seq(arr, p + 1, right);
}

static void *quicksort_par_wrapper(void *arg);

static void quicksort_par(int *arr, size_t left, size_t right) {
    if (right - left < 2) {
        return;
    }

    size_t p = partition(arr, left, right);

    if (sem_trywait(&thread_limiter) == 0) {
        ThreadArgs *thread_arg = malloc(sizeof(ThreadArgs));
        thread_arg->arr = arr;
        thread_arg->left = left;
        thread_arg->right = p + 1;

        pthread_t thread;
        if (pthread_create(&thread, NULL, quicksort_par_wrapper, thread_arg) == 0) {

            quicksort_par(arr, p + 1, right);
            pthread_join(thread, NULL);
        } else {
            sem_post(&thread_limiter);
            free(thread_arg);
            quicksort_seq(arr, left, p + 1);
            quicksort_seq(arr, p + 1, right);
        }
    } else {
        quicksort_seq(arr, left, p + 1);
        quicksort_seq(arr, p + 1, right);
    }
}


static void *quicksort_par_wrapper(void *arg) {
    ThreadArgs *thread_arg = (ThreadArgs *)arg;
    quicksort_par(thread_arg->arr, thread_arg->left, thread_arg->right);
    free(thread_arg);
    sem_post(&thread_limiter);
    return NULL;
}

static void parallel_quicksort(int *arr, size_t n, int num_threads) {
    if (num_threads <= 1) {
        quicksort_seq(arr, 0, n);
        return;
    }

    if (sem_init(&thread_limiter, 0, num_threads - 1) != 0) {
        perror("sem_init");
        quicksort_seq(arr, 0, n);
        return;
    }

    quicksort_par(arr, 0, n);

    sem_destroy(&thread_limiter);
}

static bool is_sorted(int *arr, size_t n) {
    for (size_t i = 1; i < n; ++i) {
        if (arr[i] < arr[i-1]) return false;
    }
    return true;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <num_threads>\n", argv[0]);
        return 1;
    }

    num_threads = atoi(argv[1]);
    if (num_threads < 1) num_threads = 1;

    const size_t N = 1000000;

    int *arr_par = malloc(N * sizeof(int));
    if(arr_par == NULL){
        printf("MALLOC FAULT\n");
        return -1;
    }

    int *arr_seq = malloc(N * sizeof(int));
    if(arr_seq == NULL){
        free(arr_par);
        printf("MALLOC FAULT\n");
        return -1;
    }

    int num, index = 0;
    FILE* file = fopen("data_file.txt", "r");
    if(file == NULL){
        printf("Error: no such file!\n");
        free(arr_par);
        free(arr_seq);
        return -1;
    }

    while (fscanf(file, "%d", &num) == 1 && index < 1000000) {
        arr_par[index] = num;
        arr_seq[index] = num;
        ++index;
    }
    fclose(file);

    struct timespec start, end;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    parallel_quicksort(arr_par, N, num_threads);
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    double par_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    clock_gettime(CLOCK_MONOTONIC, &start);
    quicksort_seq(arr_seq, 0, N);
    clock_gettime(CLOCK_MONOTONIC, &end);

    double seq_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Threads: %d\n", num_threads);
    printf("Parallel time:   %.3f ms\n", par_time * 1000);
    printf("Sequential time: %.3f ms\n", seq_time * 1000);
    printf("Speedup:         %.2fx\n", seq_time / par_time);
    printf("Efficienty:      %.3f\n", (seq_time / par_time) / num_threads);
    printf("Correct:         %s\n", is_sorted(arr_par, N) ? "yes" : "no");

    free(arr_par);
    free(arr_seq);
    return 0;
}
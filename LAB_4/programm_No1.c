#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include <unistd.h> // write
#include <dlfcn.h> // dlopen, dlsym, dlclose, RTLD_*

#include "include/library.h"

extern int f_prime_count(int a, int b);
extern int *f_sort(int *array, size_t n);

void print_string(const char *str) {
    write(STDOUT_FILENO, str, strlen(str));
}

int read_line(char *buffer, int max_size) {
    int bytes_read = read(STDIN_FILENO, buffer, max_size - 1);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        if (buffer[bytes_read - 1] == '\n') {
            buffer[bytes_read - 1] = '\0';
        }
    }
    return bytes_read;
}

void demonstrate_primes() {
    char buffer[128];
    int a, b;
    
    print_string("Input borders: ");
    read_line(buffer, sizeof(buffer));
    
    char *token = strtok(buffer, " ");
    if (token) a = atoi(token);
    
    token = strtok(NULL, " ");
    if (token) b = atoi(token);
    
    int count = f_prime_count(a, b);
    
    char num_str[128];
    snprintf(num_str, sizeof(num_str), "Primes Numbers in [a; b]: %d", count);
    print_string(num_str);
    print_string("\n\n");
}

void demonstrate_sort() {
    char buffer[2048];
    int array[1000];
    int n = 0;
    
    print_string("Input elements of array: ");
    read_line(buffer, sizeof(buffer));
    
    char *token = strtok(buffer, " ");
    while (token != NULL) {
        array[n++] = atoi(token);
        token = strtok(NULL, " ");
    }
    
    int *sorted = f_sort(array, n);
    
    print_string("Sorted array: ");
    for (int i = 0; i < n; i++) {
        char num_str[32];
        snprintf(num_str, sizeof(num_str), "%d ", sorted[i]);
        print_string(num_str);
    }
    print_string("\n\n");
}

void show_action() {
    print_string("Action\n");
    print_string("1. Count of Primes Numbers in [a; b]\n");
    print_string("2. Sort array\n");
    print_string("3. Exit\n");
    print_string("Choose action: ");
}

int main() {
    char buffer[32];
    
    while (1) {
        show_action();
        read_line(buffer, sizeof(buffer));
        
        int command = atoi(buffer);
        
        switch (command) {
            case 1:
                demonstrate_primes();
                break;
            case 2:
                demonstrate_sort();
                break;
            case 3:
                return 0;
                
            default:
                print_string("Unknown action. Try again!\n\n");
                break;
        }
    }
    
    return 0;
}
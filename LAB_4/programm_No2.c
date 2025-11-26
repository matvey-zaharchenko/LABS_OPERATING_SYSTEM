#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> // write
#include <dlfcn.h> // dlopen, dlsym, dlclose, RTLD_*

#include "include/library.h"

static prime_count *prime_count_func;
static sort *sort_func;

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
    
    int count = prime_count_func(a, b);
    
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
    
    int *sorted = sort_func(array, n);
    
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
    print_string("0. Switch library\n");
    print_string("1. Count of Primes Numbers in [a; b]\n");
    print_string("2. Sort array\n");
    print_string("3. Exit\n");
    print_string("Choose action: ");
}

int validate_input(int argc, char* argv[]){
    if(argc < 3){
        print_string("Error: Incorrect input\n");
        print_string("Usage: <programm_name> <path_to_lib1> <path_to_lib2>\n");
        return -1;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if(validate_input(argc, argv) == -1) return -1;

    char path1[256];
    char path2[256];
    strcpy(path1, argv[1]);
    strcpy(path2, argv[2]);
    int open_lib = 1;
    
    void *library = dlopen(path1, RTLD_LOCAL | RTLD_LAZY);
    if (library == NULL) {
        const char msg[] = "Error: can't open library\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        return -1;
    }

    prime_count_func = dlsym(library, "f_prime_count");
    if(prime_count_func == NULL){
        print_string("Failed to find prime_count implementation");
        return -1;
    }
    sort_func = dlsym(library, "f_sort");
    if(sort_func == NULL){
        print_string("Failed to find sort implementation");
        return -1;
    }

    char buffer[32];
    
    while (1) {
        show_action();
        read_line(buffer, sizeof(buffer));
        
        int command = atoi(buffer);
        
        switch (command) {
            case 0:
                dlclose(library);
                if(open_lib == 1){
                    open_lib = 2;
                    library = dlopen(path2, RTLD_LOCAL | RTLD_LAZY);
                }else if(open_lib == 2){
                    open_lib = 1;
                    library = dlopen(path1, RTLD_LOCAL | RTLD_LAZY);
                }

                if (library == NULL) {
                    const char msg[] = "Error: can't open library\n";
                    write(STDERR_FILENO, msg, sizeof(msg));
                    return -1;
                }

                prime_count_func = dlsym(library, "f_prime_count");
                if(prime_count_func == NULL){
                    print_string("Failed to find prime_count implementation");
                    return -1;
                }
    
                sort_func = dlsym(library, "f_sort");
                if(sort_func == NULL){
                    print_string("Failed to find sort implementation");
                    return -1;
                }

                print_string("Library switched!\n\n");
                break;
            case 1:
                demonstrate_primes();
                break;
            case 2:
                demonstrate_sort();
                break;
            case 3:
                dlclose(library); 
                return 0;
                
            default:
                print_string("Unknown action. Try again!\n\n");
                break;
        }
    }
    
    return 0;
}
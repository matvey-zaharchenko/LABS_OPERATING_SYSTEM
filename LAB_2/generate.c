#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[]){
    if(argc < 2){
        printf("Incorrect number of arguments\n");
        printf("Usage: %s <number_of_elements>\n", argv[0]);
        return -1;
    }

    FILE* output_file = fopen("data_file.txt", "w");

    int count = atoi(argv[1]);

    for(int i = 0; i < count; ++i){
        int n = rand() % 10000;
        fprintf(output_file, "%d\n", n);
    }
}
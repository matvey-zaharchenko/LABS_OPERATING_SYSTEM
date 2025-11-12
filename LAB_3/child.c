#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <wait.h>
#include <semaphore.h>
#include <stdio.h>

#define SHM_SIZE 4096

void intToStr(char* number, int num) {
    char temp[12];
    bool negative = false;
    int len = 0;
    
    if(num < 0) {
        num = -num;
        negative = true;
    }

    do {
        temp[len++] = (num % 10) + '0';
        num /= 10;
    } while(num > 0);

    if(negative) {
        temp[len++] = '-';
    }

    for(int i = 0; i < len; ++i) {
        number[i] = temp[len - i - 1];
    }
    number[len] = '\0';
}

int strToInt(char* number) {
    int result = 0;
    int start = 0;
    if(number[0] == '-') {
        start = 1;
    }

    for(int i = start; i < strlen(number); ++i) {
        result = result * 10 + (number[i] - '0');
    }

    if(start) {
        return -result; 
    }
    return result;
}

void compare(char* res, int num1, int num2) {
    char stringNumberOne[12];
    char stringNumberTwo[12];
    intToStr(stringNumberOne, num1);
    intToStr(stringNumberTwo, num2);

    strcpy(res, stringNumberOne);
    strcat(res, " ");
    strcat(res, stringNumberTwo);
    strcat(res, "\n");
}

int main(int argc, char **argv) {
    if (argc < 5) {
        const char error_msg[] = "error: not enough arguments\n";
        write(STDERR_FILENO, error_msg, sizeof(error_msg));
        _exit(EXIT_FAILURE);
    }

    const char *SHM_NAME = argv[1];
    const char *SEM_NAME_SERVER = argv[2];
    const char *SEM_NAME_CHILD = argv[3];
    const char *filename = argv[4];

    int shm = shm_open(SHM_NAME, O_RDWR, 0600);
    if (shm == -1) {
        const char error_msg[] = "error: failed to open SHM\n";
        write(STDERR_FILENO, error_msg, sizeof(error_msg));
        _exit(EXIT_FAILURE);
    }

    char *shm_buf = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);
    if (shm_buf == MAP_FAILED) {
        const char error_msg[] = "error: failed to map\n";
        write(STDERR_FILENO, error_msg, sizeof(error_msg));
        _exit(EXIT_FAILURE);
    }

    sem_t *sem_parent = sem_open(SEM_NAME_SERVER, O_RDWR);
    if (sem_parent == SEM_FAILED) {
        const char error_msg[] = "error: failed to open semaphore\n";
        write(STDERR_FILENO, error_msg, sizeof(error_msg));
        _exit(EXIT_FAILURE);
    }

    sem_t *sem_child = sem_open(SEM_NAME_CHILD, O_RDWR);
    if (sem_child == SEM_FAILED) {
        const char error_msg[] = "error: failed to open semaphore\n";
        write(STDERR_FILENO, error_msg, sizeof(error_msg));
        _exit(EXIT_FAILURE);
    }

    int file = open(filename, O_RDONLY);
    if (file == -1) {
        const char error_msg[] = "error: failed to open file\n";
        write(STDERR_FILENO, error_msg, sizeof(error_msg));
        _exit(EXIT_FAILURE);
    }

    if (dup2(file, STDIN_FILENO) == -1) {
        const char error_msg[] = "error: failed to redirect stdin\n";
        write(STDERR_FILENO, error_msg, sizeof(error_msg));
        _exit(EXIT_FAILURE);
    }
    close(file);

    char buf[4096];
    ssize_t bytes;
    bool running = true;

    while (running && (bytes = read(STDIN_FILENO, buf, sizeof(buf)))) {
        if (bytes < 0) {
            const char error_msg[] = "error: failed to read from file\n";
            write(STDERR_FILENO, error_msg, sizeof(error_msg));
            _exit(EXIT_FAILURE);
        }

        int numbers[3];
        int count = 0, strIndex = 0;
        char strNumber[12];
        char outputString[25];
        size_t msg_len;
        for (uint32_t i = 0; i < bytes && running; ++i) {
            if(buf[i] == '-' || (buf[i] >= '0' && buf[i] <= '9')) {
                strNumber[strIndex++] = buf[i];
            } else if(buf[i] == ' ' || buf[i] == '\n') {
                strNumber[strIndex] = '\0';
                numbers[count++] = strToInt(strNumber);

                strNumber[0] = '\0';
                strIndex = 0;

                if(count == 3 && buf[i] == '\n') {
                    if(numbers[1] == 0 || numbers[2] == 0) {
                        const char error_msg[] = "error: in line division by zero\n";
                        msg_len = strlen(error_msg);
    
                        sem_wait(sem_parent); 
                        uint32_t *length = (uint32_t *)shm_buf;
                        char *text = shm_buf + sizeof(uint32_t);

                        *length = msg_len;
                        memcpy(text, error_msg, *length);
                        sem_post(sem_child); 
                        count = 0;
                        continue;
                    }
                    
                    int result1 = numbers[0] / numbers[1];
                    int result2 = numbers[0] / numbers[2];

                    count = 0;
                    outputString[0] = '\0';
                    compare(outputString, result1, result2);
                    
                    sem_wait(sem_parent); // ждем знака от родителя, что он выполнил свою работу 
                    uint32_t *length = (uint32_t *)shm_buf;
                    char *text = shm_buf + sizeof(uint32_t);
                    
                    *length = strlen(outputString);
                    memcpy(text, outputString, *length);
                    sem_post(sem_child); // говорим родителю, что ребенок выполнил свою работу
                } else if(count < 3 && buf[i] == '\n') {
                    const char error_msg[] = "error: in line must be 3 numbers\n";
                    msg_len = strlen(error_msg);
                    
                    sem_wait(sem_parent);
                    uint32_t *length = (uint32_t *)shm_buf;
                    char *text = shm_buf + sizeof(uint32_t);

                    *length = msg_len;
                    memcpy(text, error_msg, *length);
                    sem_post(sem_child);
                }
            } else {
                const char error_msg[] = "error: in line numbers must be natural\n";
                msg_len = strlen(error_msg);

                sem_wait(sem_parent);
                uint32_t *length = (uint32_t *)shm_buf;
                char *text = shm_buf + sizeof(uint32_t);

                *length = msg_len;
                memcpy(text, error_msg, *length);
                sem_post(sem_child);
            }
        }
    }

    sem_wait(sem_parent);
    uint32_t *length = (uint32_t *)shm_buf;
    *length = UINT32_MAX;
    sem_post(sem_child);

    sem_close(sem_parent);
    sem_close(sem_child);
    munmap(shm_buf, SHM_SIZE);
    close(shm);

    return 0;
}
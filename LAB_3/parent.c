#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>
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

char SHM_NAME[1024] = "shm-name";
char SEM_NAME_PARENT[1024] = "sem-parent";
char SEM_NAME_CHILD[1024] = "sem-child";

int main(int argc, char* argv[]) {
    if(argc < 2){
        const char error_msg[] = "error: incorrect input\nUsage: <programmname> <file_name>\n";
        write(STDERR_FILENO, error_msg, sizeof(error_msg));
        _exit(EXIT_FAILURE);
    }

    char pid[64] = "\0";
    snprintf(pid, sizeof(pid), "%d", getpid());

    snprintf(SHM_NAME, sizeof(SHM_NAME), "shm-%s", pid);
    snprintf(SEM_NAME_PARENT, sizeof(SEM_NAME_PARENT), "sem-parent-%s", pid);
    snprintf(SEM_NAME_CHILD, sizeof(SEM_NAME_CHILD), "sem-child-%s", pid);

    int shm = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_TRUNC, 0600);
    if (shm == -1) {
        const char error_msg[] = "error: failed to open SHM\n";
        write(STDERR_FILENO, error_msg, sizeof(error_msg));
        _exit(EXIT_FAILURE);
    }

    if (ftruncate(shm, SHM_SIZE) == -1) {
        const char error_msg[] = "error: failed to resize SHM\n";
        write(STDERR_FILENO, error_msg, sizeof(error_msg));
        _exit(EXIT_FAILURE);
    }

    char *shm_buf = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);
    if (shm_buf == MAP_FAILED) {
        const char error_msg[] = "error: failed to map SHM\n";
        write(STDERR_FILENO, error_msg, sizeof(error_msg));
        _exit(EXIT_FAILURE);
    }

    sem_t *sem_parent = sem_open(SEM_NAME_PARENT, O_RDWR | O_CREAT | O_TRUNC, 0600, 1);
    if (sem_parent == SEM_FAILED) {
        const char error_msg[] = "error: failed to create semaphore\n";
        write(STDERR_FILENO, error_msg, sizeof(error_msg));
        _exit(EXIT_FAILURE);
    }

    sem_t *sem_child = sem_open(SEM_NAME_CHILD, O_RDWR | O_CREAT | O_TRUNC, 0600, 0);
    if (sem_child == SEM_FAILED) {
        const char error_msg[] = "error: failed to create semaphore\n";
        write(STDERR_FILENO, error_msg, sizeof(error_msg));
        _exit(EXIT_FAILURE);
    }

    pid_t child = fork();

    if (child == 0) {
        char *args[] = {"child", SHM_NAME, SEM_NAME_PARENT, SEM_NAME_CHILD, argv[1], NULL};
        execv("./child", args);

        const char error_msg[] = "error: failed to exec\n";
        write(STDERR_FILENO, error_msg, sizeof(error_msg));
        _exit(EXIT_FAILURE);
    }
    else if (child == -1) {
        const char error_msg[] = "error: failed to fork\n";
        write(STDERR_FILENO, error_msg, sizeof(error_msg));
        _exit(EXIT_FAILURE);
    }

    bool running = true;
    while (running) {
        sem_wait(sem_child); // ждем пока ребенок не даст знак, что он отработал
    
        uint32_t *length = (uint32_t *)shm_buf;
        char *text = shm_buf + sizeof(uint32_t);
        if (*length == UINT32_MAX) {
            running = false;
        }
        else if (*length > 0) {
            write(STDOUT_FILENO, text, *length);
        }
        
        sem_post(sem_parent); // подаем знак ребенку, что родитель выполнил свою часть работы
    }

    waitpid(child, NULL, 0);

    sem_close(sem_parent);
    sem_close(sem_child);
    sem_unlink(SEM_NAME_PARENT);
    sem_unlink(SEM_NAME_CHILD);
    munmap(shm_buf, SHM_SIZE);
    shm_unlink(SHM_NAME);
    close(shm);

    return 0;
}
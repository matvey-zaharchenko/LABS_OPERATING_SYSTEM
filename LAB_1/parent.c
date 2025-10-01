#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>

#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

static char CHILD_PROGRAM_NAME[] = "child";

int main(int argc, char **argv) {
	if (argc == 1) {
		char msg[1024];
		uint32_t len = snprintf(msg, sizeof(msg) - 1, "usage: %s filename\n", argv[0]);
		write(STDERR_FILENO, msg, len);
		exit(EXIT_SUCCESS);
	}

	char progpath[1024];
	{
		ssize_t len = readlink("/proc/self/exe", progpath,
		                    sizeof(progpath) - 1);
		if (len == -1) {
			const char msg[] = "error: failed to read full program path\n";
			write(STDERR_FILENO, msg, sizeof(msg));
			exit(EXIT_FAILURE);
		}

		while (progpath[len] != '/')
			--len;

		progpath[len] = '\0';
	}

	int child_to_parent[2];

	if (pipe(child_to_parent) == -1) {
		const char msg[] = "error: failed to create pipe\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
	}

	const pid_t child = fork();

	switch (child) {
	case -1: { 
		const char msg[] = "error: failed to spawn new process\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
	} break;

	case 0: {
        close(child_to_parent[0]);
        
        int32_t file = open(argv[1], O_RDONLY);
		
        dup2(file, STDIN_FILENO); 
        close(file);

        dup2(child_to_parent[1], STDOUT_FILENO); 
        close(child_to_parent[1]);

		{
			char path[1024];
			snprintf(path, sizeof(path), "%s/%s", progpath, CHILD_PROGRAM_NAME);

			char *const args[] = {CHILD_PROGRAM_NAME, NULL};

			int32_t status = execv(path, args);

			if (status == -1) {
				const char msg[] = "error: failed to exec into new exectuable image\n";
				write(STDERR_FILENO, msg, sizeof(msg));
				exit(EXIT_FAILURE);
			}
		}
	} break;

	default: { 

		close(child_to_parent[1]);

		char buf[4096];
		ssize_t bytes;

		while (bytes = read(child_to_parent[0], buf, sizeof(buf))) {
			if (bytes < 0) {
				const char msg[] = "error: failed to read from stdin\n";
				write(STDERR_FILENO, msg, sizeof(msg));
				exit(EXIT_FAILURE);
			}
			write(STDOUT_FILENO, buf, bytes);
		}

		close(child_to_parent[0]);
		wait(NULL);
	} break;
	}
}
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

void intToStr(char* number, int num){
    char temp[12];
    bool negative = false;
    int len = 0;
    
    if(num < 0){
        num = -num;
        negative = true;
    }

    do{
        temp[len++] = (num % 10) + '0';
        num /= 10;
    } while(num > 0);

    if(negative){
        temp[len++] = '-';
    }

    for(int i = 0; i < len; ++i){
        number[i] = temp[len - i - 1];
    }
    number[len] = '\0';
}

int strToInt(char* number){
    int result = 0;
    int start = 0;
    if(number[0] == '-'){
        start = 1;
    }

    for(int i = start; i < strlen(number); ++i){
        result = result * 10 + (number[i] - '0');
    }

    if(start){
        return -result; 
    }
    return result;
}

void compare(char* res, int num1, int num2){
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
	char buf[4096];
	ssize_t bytes;

    int numbers[3];
    int count = 0, strIndex = 0;
    char strNumber[12];
    char outputString[25];

	while (bytes = read(STDIN_FILENO, buf, sizeof(buf))) { // STDIN_FILENO = input_file
		if (bytes < 0) {
			const char msg[] = "error: failed to read from stdin\n";
			write(STDERR_FILENO, msg, sizeof(msg));
			exit(EXIT_FAILURE);
		}

		// Transform data
		for (uint32_t i = 0; i < bytes; ++i) {
            if(buf[i] == '-' || (buf[i] >= '0' && buf[i] <= '9')){
                strNumber[strIndex++] = buf[i];
            } else if(buf[i] == ' ' || buf[i] == '\n'){
                strNumber[strIndex] = '\0';
                numbers[count++] = strToInt(strNumber);

                strNumber[0] = '\0';
                strIndex = 0;

                if(count == 3 && buf[i] == '\n'){
                    if(numbers[1] == 0 || numbers[2] == 0){
                        const char msg[] = "error: division by zero\n";
                        write(STDERR_FILENO, msg, sizeof(msg));
                        exit(EXIT_FAILURE);
                    }
                    int result1 = numbers[0] / numbers[1];
                    int result2 = numbers[0] / numbers[2];

                    count = 0;
                    outputString[0] = '\0';
                    compare(outputString, result1, result2);
                    write(STDOUT_FILENO, outputString, strlen(outputString));
                } else if(count < 3 && buf[i] == '\n'){
                    const char msg[] = "error: in line must be 3 numbers\n";
                    write(STDERR_FILENO, msg, sizeof(msg));
                    exit(EXIT_FAILURE);
                }
            }else{
                const char msg[] = "error: numbers must be natural\n";
                write(STDERR_FILENO, msg, sizeof(msg));
                exit(EXIT_FAILURE);
            }
		}
	}
    return 0;
}
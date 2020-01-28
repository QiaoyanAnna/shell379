#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define LINE_LENGTH 100
#define MAX_ARGS 7
#define MAX_LENGTH 20
#define MAX_PT_ENTRIES 32

int validateCmd(char line[LINE_LENGTH+2], char cmd[MAX_ARGS][MAX_LENGTH]) {
    int i = 0, ch = 0, arg = 0;
    int lineLen = strlen(line);
    // find the position of &
    char *p;
    int idx;
    p = strchr(line, '&');
    idx = (int)(p - line);
    // check if & is the last argument
    if (p != NULL) {
        if (lineLen-2 != idx) {
            printf("Invalid Command: & must be the last argument.\n");
            return -1;
        } else if (line[lineLen-3] != ' ') {
            printf("Invalid Command: & must be the last argument.\n");
            return -1;
        }
    }
    // check if the input line ends with a newline to see 
    // if it exceeds the maximum number of characters
    if (line[lineLen-1] != '\n') {
        printf("Invalid Command: the number of characters in the input line is more than 100.\n");
        return -1;
    }
    // find the first character
    int idxOfFirstChar = 0;
    while (line[idxOfFirstChar] == ' ') {
        idxOfFirstChar++;
    }

    for(i = idxOfFirstChar; i < lineLen; i++) {
        if (line[i] == ' ') {
            cmd[arg][ch]='\0';
            arg++;
            if (arg > MAX_ARGS) {
                printf("Invalid Command: the number of arguments in the command is more than 7.\n");
                return -1;
            }
            ch = 0;
            while (line[i+1] == ' ') {
                i++;
            }
        } else if (line[i] == '\n') {
            cmd[arg][ch]='\0';
            if (line[i-1] != ' '){
                arg++;
                if (arg > MAX_ARGS) {
                    printf("Invalid Command: the number of arguments in the command is more than 7.\n");
                    return -1;
                }
            }
        } else {
            cmd[arg][ch] = line[i];
            ch++;
            if (ch > MAX_LENGTH) {
                printf("Invalid Command: the number of characters in an argument is more than 20.\n");
                return -1;
            }
        }
    }
    return arg;   
}

void jobs() {
    printf("jobs\n");
}


int main(int argc, char *argv[]) {
    bool exit = false;
    while (!exit) {
        char line[LINE_LENGTH+2]; // one space for new line, one space for \0
        char cmd[MAX_ARGS][MAX_LENGTH];
        int numOfArg = 0;

        fflush(stdin);
        printf("SHELL379: ");
        fgets(line, sizeof line, stdin);

        int ret = validateCmd(line, cmd);
        if (ret == -1) {
            continue;
        } else {
            numOfArg = ret;
        }

        // printf("numOfArg: %d\n", numOfArg);
        // for (int i = 0; i < numOfArg; i++) {
        //     printf("arg %d: %s\n", i+1, cmd[i]);
        // }

        // command jobs is entered
        if (strcmp(cmd[0],"jobs") == 0) {
            if (numOfArg != 1) {
                printf("Invalid Command: jobs does not take any argument.\n");
                continue;
            } else {
                jobs();
            }
        }


    }
    return 0;
}

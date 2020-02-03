#include <stdio.h>
#include <string.h>

#include "shell379.h"


// validate command
// split the whole line into separate command and argument 
// and store in a 2D array cmd[MAX_ARGS][MAX_LENGTH]
int validateCmd(char line[LINE_LENGTH+2], char cmd[MAX_ARGS][MAX_LENGTH]) {
    int i = 0, ch = 0, arg = 0;
    int lineLen = strlen(line);
    // find the position of &
    char *p;
    p = strchr(line, '&');

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

    // start with the first character and loop until the end of the line
    for(i = idxOfFirstChar; i < lineLen; i++) {
        if (line[i] == ' ') { // get rid of the space
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

    // check if & is the last argument
    if (p != NULL) {
        if (strcmp(cmd[arg-1], "&") != 0) {
            printf("Invalid Command: & must be the last argument.\n");
            return -1;
        } 
    }

    // fix line into normal command by adding one space between each argument
    memset(line, '\0', lineLen);
    strcpy(line, cmd[0]);
    for (int j = 1; j < arg; j++) {
        strcat(line, " ");
        strcat(line, cmd[j]);
    }
    return arg;   
}

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define LINE_LENGTH 100
#define MAX_ARGS 7
#define MAX_LENGTH 20
#define MAX_PT_ENTRIES 32

bool readCmd() {
    char cmd[MAX_ARGS][MAX_LENGTH];
    char line[LINE_LENGTH+2]; // one space for new line, one space for \0
    int i = 0, ch = 0, arg = 0;
    printf("SHELL379: ");
    fgets(line, sizeof line, stdin);	
    int lineLen = strlen(line);
    if (line[lineLen-1] != '\n') {
        return false;
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
                return false;
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
                    return false;
                }
            }
        } else {
            cmd[arg][ch] = line[i];
            ch++;
            if (ch > MAX_LENGTH) {
                return false;
            }
        }
    }
    int numOfArg = arg;
    printf("numOfArg: %d\n\n", numOfArg);
    for (i = 0; i < numOfArg; i++) {
        printf("arg %d: %s\n", i+1, cmd[i]);
    }
    return true;   
}

int main(int argc, char *argv[]) {
    bool exit = false;
    while (!exit) {
        exit = readCmd();
    }
    return 0;
}

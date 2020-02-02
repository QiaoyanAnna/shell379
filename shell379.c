#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h> 
#include <stdint.h>

#define LINE_LENGTH 100
#define MAX_ARGS 7
#define MAX_LENGTH 20
#define MAX_PT_ENTRIES 32

struct process 
{
    int pid;
    char s;
    int sec;
    char command[100];
};

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

    // check if & is the last argument
    if (p != NULL) {
        if (strcmp(cmd[arg-1], "&") != 0) {
            printf("Invalid Command: & must be the last argument.\n");
            return -1;
        } 
    }

    memset(line, '\0', lineLen);
    strcpy(line, cmd[0]);
    for (int j = 1; j < arg; j++) {
        strcat(line, " ");
        strcat(line, cmd[j]);
    }
    return arg;   
}

void endExecution() {
    printf("exit\n");
}

void displayStatus(int numOfProc, struct process proc[MAX_PT_ENTRIES]) {
    printf("Running processes:\n");
    if (numOfProc > 0) {
        printf("#\tPID\tS\tSEC\tCOMMAND\n");
        for (int i = 0; i < numOfProc; i++) {
            printf("%d:\t%d\t%c\t%d\t%s\n", i, proc[i].pid, proc[i].s, proc[i].sec, proc[i].command);
        }
    }
    printf("Processes =\t%d active\n", numOfProc);
    printf("Completed processes:\n");
    printf("User time =\t0 seconds\n");
    printf("Sys  time =\t0 seconds\n");
}

int killProc(int pid, int numOfProc, char cmd[MAX_ARGS][MAX_LENGTH], struct process proc[MAX_PT_ENTRIES]) {
    int ret;
    ret = kill(pid, SIGKILL);

    int killInd = 0;
    if (ret == 0) {
        // find the index of the killed process
        for (int i = 0; i < numOfProc; i++) {
            if (proc[i].pid == pid) {
                killInd = i;
                break;
            }
        }

        for (int j = killInd; j < numOfProc; j++) {
            proc[j].pid = proc[j+1].pid;
            proc[j].s = proc[j+1].s;
            proc[j].sec = proc[j+1].sec;
            strcpy(proc[j].command, proc[j+1].command);
        }

    } else {
        printf("An error occured while killing the process %d\n", pid);
    }
    return ret;
}

int resumeSuspend(int pid, int numOfProc, char cmd[MAX_ARGS][MAX_LENGTH], struct process proc[MAX_PT_ENTRIES]) {
    int ret;
    if (strcmp(cmd[0],"resume") == 0) {
        ret = kill(pid, SIGCONT);
        if (ret == 0) {
            for (int i = 0; i < numOfProc; i++) {
                if (proc[i].pid == pid) {
                    proc[i].s = 'R';
                    break;
                }
            }
        }   
    } else {
        ret = kill(pid, SIGSTOP);
        if (ret == 0) {
            for (int i = 0; i < numOfProc; i++) {
                if (proc[i].pid == pid) {
                    proc[i].s = 'S';
                    break;
                }
            }
        }
    }

    return ret;
}

int main(int argc, char *argv[]) {
    bool isExit = false;
    int numOfProc = 0;
    while (!isExit) {
        char line[LINE_LENGTH+2]; // one space for new line, one space for \0
        char cmd[MAX_ARGS][MAX_LENGTH];
        struct process proc[MAX_PT_ENTRIES];
        int numOfArg = 0;
        int ret;
        
        fflush(stdin);
        printf("SHELL379: ");
        fgets(line, sizeof line, stdin);

        ret = validateCmd(line, cmd);
        if (ret == -1) {
            continue;
        } else {
            numOfArg = ret;
        }

        // printf("numOfArg: %d\n", numOfArg);
        // for (int i = 0; i < numOfArg; i++) {
        //     printf("arg %d: %s\n", i+1, cmd[i]);
        // }
        // printf("line: %s\n", line);

        // command exit or jobs is entered 
        if (strcmp(cmd[0],"exit") == 0 || strcmp(cmd[0],"jobs") == 0) {
            if (numOfArg != 1) {
                printf("Invalid Command: %s does not take any argument.\n", cmd[0]);
                continue;
            }
            if (strcmp(cmd[0],"exit") == 0) {
                endExecution();
                break;
            } else if (strcmp(cmd[0],"jobs") == 0) {
                displayStatus(numOfProc, proc);
            }
        }
        // command kill, reusme, sleep, suspend or wait is entered
        else if (strcmp(cmd[0],"kill") == 0 || strcmp(cmd[0],"resume") == 0 || strcmp(cmd[0],"sleep") == 0
            || strcmp(cmd[0],"suspend") == 0 || strcmp(cmd[0],"wait") == 0) {  
            if (numOfArg != 2) {
                printf("Invalid Command: %s takes one argument.\n", cmd[0]);
                continue;
            } 

            int cmdInt = atoi(cmd[1]);
            int numOfDigits = floor (log10 (abs (cmdInt))) + 1;
            int lenOfCmd = strlen(cmd[1]);

            // check if valid
            if (cmdInt == 0 && cmd[1][0] != '0') {
                printf("Invalid Command: the argument should be an integer.\n");
                continue;
            } else if (numOfDigits != lenOfCmd) {
                printf("Invalid Command: the argument should be an integer.\n");
                continue;
            } 

            if (strcmp(cmd[0],"sleep") == 0) {
                sleep(cmdInt);
                continue;
            } else if (strcmp(cmd[0],"wait") == 0) {
                ret = waitpid(cmdInt, NULL, 0);
                if (ret == -1) {
                    printf("An error occured while waiting the process %d\n", cmdInt);
                    continue;
                }
            } else if (strcmp(cmd[0],"kill") == 0) {
                ret = killProc(cmdInt, numOfProc, cmd, proc);
                if (ret == 0) {
                    numOfProc--;
                } else {
                    continue;
                }
            } else {
                ret = resumeSuspend(cmdInt, numOfProc, cmd, proc);
                if (ret != 0) {
                    printf("An error occured while %sing the process %d\n", cmd[0], cmdInt);
                    continue;
                }
            }
            
        }
        // other command
        else {
            int rc = fork();
            if (rc < 0) { // fork failed; exit
                fprintf(stderr, "fork failed\n");
                _exit(1);
            } else if (rc == 0) { // child (new process)
                // printf("Child pid: %d\n", (int) getpid());
                char *tmp[numOfArg + 1];
                for (int j = 0; j < numOfArg; j++){
                    *(tmp+j) = cmd[j];
                }
                *(tmp + numOfArg) = NULL;
                if(execvp(tmp[0], tmp) < 0) {
                    perror( "Exec problem:" );
                }
                _exit(0);
            } else { // parent goes down this path (original process)
                // printf("Parent pid: %d\n", (int) getpid());
                if (strcmp(cmd[numOfArg-1],"&") == 0) {
                    // printf("Child pid: %d\n", rc);
                    numOfProc++;
                    proc[numOfProc-1].pid = rc;
                    proc[numOfProc-1].s = 'R';
                    proc[numOfProc-1].sec = 0;
                    strcpy(proc[numOfProc-1].command, line);
                } else {
                    waitpid(rc, NULL, 0);
                }               
            }
        }
    }
    return 0;
}

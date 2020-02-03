#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h> 

#include "shell379.h"
#include "validateCmd.h"
#include "helper.h"
#include "buildInCmd.h"


int main(int argc, char *argv[]) {
    bool isExit = false;
    int numOfProc = 0;
    while (!isExit) {
        char line[LINE_LENGTH+2]; // one space for new line, one space for \0
        char cmd[MAX_ARGS][MAX_LENGTH];
        struct process proc[MAX_PT_ENTRIES];
        int numOfArg = 0;
        int ret;
        
        // get the command
        fflush(stdin);
        printf("SHELL379: ");
        fgets(line, sizeof line, stdin);

        ret = validateCmd(line, cmd);
        if (ret == -1) {
            continue;
        } else {
            numOfArg = ret;
        }

        // command exit or jobs is entered 
        if (strcmp(cmd[0],"exit") == 0 || strcmp(cmd[0],"jobs") == 0) {
            if (numOfArg != 1) {
                printf("Invalid Command: %s does not take any argument.\n", cmd[0]);
                continue;
            }
            if (strcmp(cmd[0],"exit") == 0) {
                bool ifAllKilled;
                ifAllKilled = endExecution(numOfProc, proc);
                if (ifAllKilled) {
                    break;
                } else {
                    continue;
                }
            } else {
                numOfProc = displayStatus(numOfProc, proc);
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
                } else {
                    sortProc(cmdInt, numOfProc, proc);
                    numOfProc--;
                }
            } else if (strcmp(cmd[0],"kill") == 0) {
                ret = killProc(cmdInt, numOfProc, proc);
                if (ret == 0) {
                    numOfProc--;
                } else {
                    continue;
                }
            } else {
                ret = resumeSuspend(cmdInt, numOfProc, cmd, proc);
                if (ret == cmdInt) {
                    printf("Process %d has already finished\n", cmdInt);
                    numOfProc--;
                } else if (ret != 0) {
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
                if (strcmp(cmd[numOfArg-1],"&") == 0) {
                    numOfProc++;
                    proc[numOfProc-1].pid = rc;
                    proc[numOfProc-1].s = 'R';
                    strcpy(proc[numOfProc-1].command, line);
                } else {
                    waitpid(rc, NULL, 0);
                }               
            }
        }
    }
    return 0;
}

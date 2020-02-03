#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/resource.h>
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

int calTime(int pid) {
    char pidChar[6]; 
    char path[20];
    
    sprintf(pidChar, "%d", pid); 
    strcpy(path, "/proc/");
    strcat(path, pidChar);
    strcat(path, "/stat");
    // printf("path: %s\n", path);

    char content[800];
    FILE *fptr;
    if ((fptr = fopen(path, "r")) == NULL) {
        return -1;
    }
    // reads text until newline is encountered
    fscanf(fptr, "%[^\n]", content);
    int i = 0;
    char *element = strtok(content, " ");
    char *arr[60];

    while (element != NULL)
    {
        arr[i++] = element;
        element = strtok (NULL, " ");
    }

    int utime = atoi(arr[13]);
    int stime = atoi(arr[14]);
    int cutime = atoi(arr[15]);
    int cstime =atoi(arr[16]);
    // printf("utime: %d, stime: %d, cutime: %d, cstime: %d\n", utime, stime, cutime, cstime);
    int totalTime = (utime + stime + cutime + cstime) / sysconf(_SC_CLK_TCK);
    // printf("total Time: %d\n", totalTime);
    fclose(fptr);

    return totalTime;
}

void sortProc (int pid, int numOfProc, struct process proc[MAX_PT_ENTRIES]) {
    int killInd = 0;

    for (int i = 0; i < numOfProc; i++) {
        if (proc[i].pid == pid) {
            killInd = i;
            break;
        }
    }

    for (int j = killInd; j < numOfProc; j++) {
        proc[j].pid = proc[j+1].pid;
        proc[j].s = proc[j+1].s;
        strcpy(proc[j].command, proc[j+1].command);
    }
}

bool endExecution(int numOfProc, struct process proc[MAX_PT_ENTRIES]) {
    int ret;
    bool ifAllKilled = true;
    for (int i = 0; i < numOfProc; i++) {
        ret = waitpid(proc[i].pid, NULL, WNOHANG);
        if (ret == 0) {
            ret = kill(proc[i].pid, SIGKILL);
            if (ret != 0) {
                printf("An error occured while killing the process %d\n", proc[i].pid);
                ifAllKilled = false;
            }
        }
    }
    return ifAllKilled;
}

int displayStatus(int numOfProc, struct process proc[MAX_PT_ENTRIES]) {

    int ret;
    int sec = 0;
    printf("Running processes:\n");
    if (numOfProc > 0) {
        for (int i = 0; i < numOfProc; i++) {
            ret = waitpid(proc[i].pid, NULL, WNOHANG);
            if (ret == proc[i].pid) {
                // the process finished
                for (int j = i; j < numOfProc; j++) {
                    proc[j].pid = proc[j+1].pid;
                    proc[j].s = proc[j+1].s;
                    strcpy(proc[j].command, proc[j+1].command);
                }
                numOfProc--;
                i--;
            } else if (i == 0) {
                ret = calTime(proc[i].pid);
                if (ret != -1) {
                    sec = ret;
                }
                printf("#\tPID\tS\tSEC\tCOMMAND\n");
                printf("%d:\t%d\t%c\t%d\t%s\n", i, proc[i].pid, proc[i].s, sec, proc[i].command);
            } else {
                ret = calTime(proc[i].pid);
                if (ret != -1) {
                    sec = ret;
                }
                printf("%d:\t%d\t%c\t%d\t%s\n", i, proc[i].pid, proc[i].s, sec, proc[i].command);
            }
        }
    }
    struct rusage ru;
    getrusage(RUSAGE_CHILDREN, &ru);
    printf("Processes =\t%d active\n", numOfProc);
    printf("Completed processes:\n");
    printf("User time =\t%ld seconds\n", ru.ru_utime.tv_sec);
    printf("Sys  time =\t%ld seconds\n", ru.ru_stime.tv_sec);
    return numOfProc;
}

int killProc(int pid, int numOfProc, struct process proc[MAX_PT_ENTRIES]) {
    int ret;
    ret = kill(pid, SIGKILL);
    if (ret == 0) {
        sortProc(pid, numOfProc, proc);
    } else {
        printf("An error occured while killing the process %d\n", pid);
    }
    return ret;
}

int resumeSuspend(int pid, int numOfProc, char cmd[MAX_ARGS][MAX_LENGTH], struct process proc[MAX_PT_ENTRIES]) {
    int ret;
    ret = waitpid(pid, NULL, WNOHANG);
    if (ret == pid){
        sortProc(pid, numOfProc, proc);
        return ret;
    }

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
                    strcpy(proc[numOfProc-1].command, line);
                } else {
                    waitpid(rc, NULL, 0);
                }               
            }
        }
    }
    return 0;
}

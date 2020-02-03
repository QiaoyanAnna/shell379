#define _POSIX_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "shell379.h"
#include "helper.h"


// end the execution when exit is enter by the user
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

// display the status when user entered command jobs
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
            } else if (i == 0) { // the first progress
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
    // get the user time and the system time 
    struct rusage ru;
    getrusage(RUSAGE_CHILDREN, &ru);
    printf("Processes =\t%d active\n", numOfProc);
    printf("Completed processes:\n");
    printf("User time =\t%ld seconds\n", ru.ru_utime.tv_sec);
    printf("Sys  time =\t%ld seconds\n", ru.ru_stime.tv_sec);
    return numOfProc;
}

// kill the process given pid
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

// resume of suspend the progress given pid
int resumeSuspend(int pid, int numOfProc, char cmd[MAX_ARGS][MAX_LENGTH], struct process proc[MAX_PT_ENTRIES]) {
    int ret; 
    // check if the progress has already finished
    ret = waitpid(pid, NULL, WNOHANG);
    if (ret == pid){
        sortProc(pid, numOfProc, proc);
        return ret;
    }

    // change the status to R(running) when the command is resume
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
    } else { // change the status to S(suspend) when the command is suspend
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
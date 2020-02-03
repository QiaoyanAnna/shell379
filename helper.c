#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shell379.h"


// calculate the CPU time for the progress given pid
int calTime(int pid) {
    char pidChar[6]; 
    char path[20];
    
    // path
    sprintf(pidChar, "%d", pid); 
    strcpy(path, "/proc/");
    strcat(path, pidChar);
    strcat(path, "/stat");

    char content[800];
    FILE *fptr;
    if ((fptr = fopen(path, "r")) == NULL) {
        return -1;
    }
    // reads text until the newline
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
    int totalTime = (utime + stime + cutime + cstime) / sysconf(_SC_CLK_TCK);
    
    fclose(fptr);

    return totalTime;
}

// delete the progress given pid in the process array
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
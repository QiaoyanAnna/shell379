#ifndef SHELL379_H
#define SHELL379_H

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

#endif


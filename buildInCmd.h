#ifndef BUILDIN_CMD_H
#define BUILDIN_CMD_H

bool endExecution(int numOfProc, struct process proc[MAX_PT_ENTRIES]);
int displayStatus(int numOfProc, struct process proc[MAX_PT_ENTRIES]);
int killProc(int pid, int numOfProc, struct process proc[MAX_PT_ENTRIES]);
int resumeSuspend(int pid, int numOfProc, char cmd[MAX_ARGS][MAX_LENGTH], struct process proc[MAX_PT_ENTRIES]);

#endif
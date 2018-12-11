//Daniel Beeman, dbeeman, CIS 415 Project 1
//This is my own work.
#ifndef P1_H
#define P1_H
#include "p1fxns.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct PCB {
	char *cmd;
	char **args;
	pid_t PID;
	int status;
};



#endif


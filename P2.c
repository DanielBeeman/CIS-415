//Daniel Beeman, dbeeman, CIS 415 Project 1
//This is my own work except that one small piece is taken from online, which is commented. 
#include <stdio.h>
#include <stdlib.h>
#include "p1fxns.h"
#include "P1.h"
#include <signal.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


struct PCB *pcb = NULL;
int Run = 0;

void sigusr1_handler(int signum)
{
	printf("Enter: %s\n",__FUNCTION__);
	Run =1;
	printf("Exit: %s\n",__FUNCTION__);
}


void freeAll (int lc){
	int x;
	for (x = 0; x < lc; x++){
		int itr = 0;
		free(pcb[x].cmd);
		for(itr = 0; pcb[x].args[itr] != NULL; itr++){
			free(pcb[x].args[itr]);
		}
		free(pcb[x].args);

	}
	free(pcb);
	pcb = NULL;
}

void WaitAndExecOrCleanup(struct PCB *prgm, int lines)
{
	printf("Enter: %s\n",__FUNCTION__);

	while(Run ==0)
	{
		usleep(1);
	}
	// TODO: Run Execvp
	int i;
	for(i = 0; i<lines; i++){
		execvp(pcb[i].cmd, pcb[i].args);
		printf("error command is: %s\n", pcb->cmd);
		freeAll(lines);
		exit(-1);
	}
	// Cleanup PCBS
	// Exit
	
	printf("Exit: %s\n",__FUNCTION__);

}

int LineCount(char *filename){
	int count = 0;
	FILE *in;
	in = fopen(filename, "r");	

	if (in==NULL) {
		printf("Unable to open file");
		return 0;
	}
	
	//from: https://www.geeksforgeeks.org/c-program-count-number-lines-file/
	char c;
	for (c = getc(in); c!= EOF; c = getc(in)){
		if (c== '\n'){
			count += 1;
		}
	}
	fclose(in);
	return count;

}

int WordCount(char *input){
	int count = 0;
	char hold[1024];
	int i = 0;
		while (i != -1){
			i = p1getword(input, i, hold);
			count += 1;			
		}
	count -= 1;
	return count;
}


void BuildPCBs(char *input, int lines){
	int i;
	char lin[1024];
		
	FILE* in;
	in = fopen(input, "r");

	//Office hour code
	pcb = (struct PCB *)malloc(sizeof(struct PCB) * lines);

	for (i=0; i<lines; i++){
		char buffer[1024];
		pcb[i].cmd = NULL;
		pcb[i].args = NULL;
		fgets(lin, sizeof(lin), in);
		lin[strcspn(lin, "\n")] = 0;
		
	
		
		char arg[1024];
		pcb[i].PID = -1;
		pcb[i].status = 0;
	
	//open file, count number of lines. Initialize the pcbs I will need. Create a 
	//loop to malloc space for args and command for each individual pcb.
		int t = 0;
		t = p1getword(lin, t, buffer);

		
		pcb[i].cmd = strdup(buffer);
		//printf("command value: %s\n", pcb[i].cmd);
		int num_words = WordCount(lin);
		pcb[i].args = (char**)malloc(sizeof(char *) * (num_words + 1));	
		int j;
		j = 0;
		int p;
		p = 0;
		while (p != -1){
			p = p1getword(lin, p, arg);
			if (p == -1){break;}
			arg[strcspn(arg, "\n")] = 0;
			
			pcb[i].args[j] = strdup(arg);
			//printf("argument is: %s\n", pcb[i].args[j]);
			j += 1;
		}
		pcb[i].args[j] = NULL;

	}
	fclose(in);

}

void LaunchPCBs(int lines) {
	int i=0;
	signal(SIGUSR1, sigusr1_handler);
	for (i=0; i<lines; i++){

		pcb[i].PID = fork();
		if (pcb[i].PID < 0) {
			printf("error, processes id < 0\n");
			exit(-1);
		}
		if (pcb[i].PID == 0){
			WaitAndExecOrCleanup(&pcb[i], lines); //might need to change...
		}

	}
	for (i = 0; i < lines; i++){
		kill(pcb[i].PID, SIGUSR1);
	}
	for (i = 0; i < lines; i++){
		kill(pcb[i].PID, SIGSTOP);
	}
	for (i = 0; i < lines; i++) {
		kill(pcb[i].PID, SIGCONT);
	}

}

void WaitPCBs(int lines){
	int i;
	i = 0;
	for (i = 0; i<lines; i++){
		wait(NULL);
	}
}

//need to change word count to count words in an input STRING
//then strip newline character and pass in string to function



int main (int argc,char* argv[]) {
	int li = LineCount(argv[1]);
	BuildPCBs(argv[1], li);
	LaunchPCBs(li);
	WaitPCBs(li);
	freeAll(li);
	return 0;
};

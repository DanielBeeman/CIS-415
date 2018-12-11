//Daniel Beeman, dbeeman, CIS 415 Project 0, This is my own work except for the cited newline issue, which is from stack overflow
#include <stdio.h>
#include <stdlib.h>
#include "anagram.h"

int main(int argc, char *argv[]){
	if(argc > 3)
	{
		exit(0);
	}
	
	FILE *input=NULL, *output=NULL;

	if (argc >= 2)
	{
		input = fopen(argv[1], "r");
	}
	else 
	{
		input = stdin;
	}

	if(input ==NULL)
	{
		exit(0);
	}
	if (argc == 3)
	{
		output = fopen(argv[2], "w");
	}
	else{
		output = stdout;
	}
	if(output == NULL)
	{
		exit(0);
	}
	char buffer[1024];
	
	struct AnagramList *head = NULL;

	while( fgets(buffer, sizeof(buffer), input) != NULL)
	{
		//found a way to remove newline character from https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input
		buffer[strcspn(buffer, "\n")] = 0;
		AddWordAList(&head, buffer);	
	}
	PrintAList(output, head);	
	FreeAList(&head);

	fclose(input);
	fclose(output);

	return 0;
}

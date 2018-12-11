//Daniel Beeman, dbeeman, CIS 415 Project 0, This is my own work except the cited qsort() function.
#include <stdio.h>
#include <string.h>
#include "anagram.h"

struct StringList *MallocSList(char *word){
	struct StringList *slist = (struct StringList *)malloc(sizeof(struct StringList));
	char *copy = strdup(word);
	slist->Word = copy;
	slist->Next = NULL;

	return slist;
};

void AppendSList(struct StringList **head, struct StringList *node){
	//Append to the string list within the family
	while(*head != NULL){
		head = &((*head)->Next);
	};
	*head = node;
};

void FreeSList(struct StringList **node){
	//Free the string list within the family
	 if (*node != NULL){
		FreeSList(&(*node)->Next);
		free((*node)->Word);
		free(*node);	
	};
	};

void PrintSList(FILE *file,struct StringList *node){
	//print the string list, Wonder if I need *node instead of node to access value
	while (node != NULL){
		fprintf(file, "\t%s\n", node->Word);
		node = node -> Next;
	}	
}

int SListCount(struct StringList *node){
	//count the number of words in the string list
	int count = 0;
	while (node != NULL){
		count += 1;
		node = node -> Next;
	}
	return count;
}

int cmpfunc (const void *a, const void *b) {
	return ( *(char*)a - *(char*)b);
};

struct AnagramList* MallocAList(char *word){
	//Malloc memory for the anagram list
	struct AnagramList* alist = (struct AnagramList*)malloc(sizeof(struct AnagramList));
	alist -> Words = MallocSList(word);
	alist -> Next = NULL;
	char *copy;
	copy = strdup(word);
	alist -> Anagram = copy;
	

	//lowercase
	int i;
	for (i = 0;i < strlen(alist -> Anagram); i++){
		alist -> Anagram[i] = tolower(alist -> Anagram[i]); 
	};
	//from https://www.tutorialspoint.com/c_standard_library/c_function_qsort.htm
	qsort(alist -> Anagram, strlen(alist -> Anagram), sizeof(char), cmpfunc);
	//free(copy);

	return alist;
	
};

void FreeAList(struct AnagramList **node){
	//Free the anagram list from memory
	if (*node != NULL) {
		//not sure I can call FreeSList because type is of AnagramList
		FreeAList(&(*node)->Next);
		FreeSList(&(*node)->Words);
		//free((*node)->Words);
		free((*node) -> Anagram);	
		free(*node);

	};
};

void PrintAList(FILE *file,struct AnagramList *node){
	//print the anagrams
	while (node != NULL){
		int count = SListCount(node->Words);
		if (count > 1){
			fprintf(file, "%s:%d\n", node->Anagram, count);
			PrintSList(file, node->Words);
		}
		node = node -> Next;
	}	
}


void AddWordAList(struct AnagramList **node, char *word){
	//Add word to the anagram list
	char *comparator;
	comparator = strdup(word);
	int i = 0;
	for (i = 0;i < strlen(comparator); i++){
		comparator[i] = tolower(comparator[i]); 
	};
	
	//sort, passing in the address
	qsort(comparator, strlen(comparator), sizeof(char), cmpfunc);
	
	if((*node) == NULL) {
		*node = MallocAList(word);
		free(comparator);
		return;
	};
	while (*node != NULL){
		if (strcmp(comparator, ((*node) -> Anagram)) == 0) {
			//strings are equal
			struct StringList *hd = MallocSList(word);
			AppendSList(&(*node) -> Words, hd);
			break;  
		} else {
			if ((*node)-> Next == NULL){
				(*node)->Next = MallocAList(word);
				break;
			} else {
				node = &(*node) -> Next;
			}

		}
	};
	free(comparator);

};


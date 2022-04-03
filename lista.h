#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

typedef struct request{
	char *source;
	char *timeStamp;
	char *payload;
}request;

typedef struct list{
	request *data;
	struct list *next, *prev;
}list;

typedef struct listHead{
	int listSize;
	list *first,*last;
}listHead;

void removeReq(listHead *head, int delete);
void insertReq(listHead *head, request *node);
int  printReqs(listHead *head);

void removeReq(listHead *head, int delete){
	list *aux = head->first;
	int i = 1;
	
	if(delete > 0){
		for(;aux != NULL && i <= delete; aux = aux->next,i++){
			
			if(i == delete){
				//If the only node.	
				if(aux->prev == NULL && aux->next == NULL){
					head->first = NULL;
					head->last = NULL;
				
				//If removing the first one.
				}else if(aux->prev == NULL){
					aux->next->prev = NULL;
					head->first = aux->next;

				//If removing the last one.
				}else if(aux->next == NULL){
					aux->prev->next = NULL;
					head->last = aux->prev;

				//If removing one in the middle.
				}else{
					aux->prev->next = aux->next;
					aux->next->prev = aux->prev;
				}

				free(aux->data);
				free(aux);
				head->listSize--;
			}
		}
  }
}

void insertReq(listHead *head, request *node){
	list *new = malloc(sizeof(list));
	new->data = node;
	new->next = NULL;
	new->prev = NULL;

  if(head->first ==  NULL){
    head->first = new;
    head->last = new;
  
	}else{
	  head->last->next = new;
    new->prev = head->last;
    head->last = new;
  }
	
	head->listSize++;
}


int printReqs(listHead *head){
	int i = 1;
	list *aux;
		
	for(aux = head->first; aux != NULL; aux = aux->next, i++){
		printf("\nNo: %d", i);
		printf("\n Source : %s", aux->data->source);
		printf("\n Time Requested: %s", aux->data->timeStamp);
		printf("\n Message: %s", aux->data->payload);
	}
  
	return i;
}

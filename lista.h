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
	int listSize = head->listSize;
	
	if(delete<= (listSize-1)&&delete>0){
		for(;aux != NULL&&i<= delete; aux = aux->next,i++){
			if(i == delete){
				if(aux->prev == NULL && aux->next == NULL){
					head->first = NULL;
					head->last = NULL;
				}else if(aux->prev == NULL){
					aux->next->prev = NULL;
					head->first = aux->next;
				}else if(aux->next == NULL){
					aux->prev->next = NULL;
					head->last = aux->prev;
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
  if(head->first ==  NULL){
    head->first = new;
    head->last = new;
  }else{
	  head->first->prev = new;
    new->next = head->first;
    head->first = new;
  }
	printf("\ninsertReq\n");
	printf("head->listSize: %d", head->listSize);
	printf("head->first->data->source: %s", head->first->data->source);
	printf("head->first->data->payload: %s", head->first->data->payload);
	printf("\ninsertReq\n");
	head->listSize++;
}

int printReqs(listHead *head){
	int i = 1;
	list *aux;
	for(aux = head->first;aux != NULL;aux = aux->next,i++){
		printf("\n|------------------------------------------|\n");
		printf(" No: %d", i);
		printf("\n Source : %s", aux->data->source);
		printf("\n Time Requested: %s", aux->data->timeStamp);
	}
  return i;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

typedef struct info{
	int idade;
	char* nome;
}info;

typedef struct list{
	info *data;
	struct list *next, *prev;
}list;

typedef struct head{
	list *first,*last;
}head;

void remover(head *cabeca);
void limpastdin();
void insert(head *cabeca,info *pessoa,list *lista);
info *criainfo();
int printar(head *cabeca);

void limpastdin(){
  char c;
  while((c = getchar()) !=  '\n' && c !=  EOF);
}

void insert(head *cabeca,info *pessoa,list *lista){
	list *novo = malloc(sizeof(list));
	novo->data = pessoa;
	list *aux;
  if(cabeca->first ==  NULL){
    cabeca->first = novo;
    cabeca->last = novo;
  }else{
	  cabeca->first->prev = novo;
    novo->next = cabeca->first;
    cabeca->first = novo;
  }
}

info *criainfo(){
	info *novo = malloc(sizeof(info));
	limpastdin();
	printf("\nDigite seu nome(em letras maiúsculas): ");
	scanf("%s",&novo->nome);
	printf("Digite sua idade: ");
	limpastdin();
	scanf("%d",&novo->idade);
	return novo;
}

int printar(head *cabeca){
	int i = 1;
	list *aux;
	for(aux = cabeca->first;aux != NULL;aux = aux->next,i++){
		printf("\n|------------------------------------------|\n");
		printf(" Código: %d", i);
		printf("\n Nome : %s", aux->data->nome);
		printf("\n Idade :%d", aux->data->idade);
	}
  return i;
}

void remover(head *cabeca){
	list *aux = cabeca->first;
	int delete,i = 1,tamlista;
	tamlista = printar(cabeca);
	printf("\nDigite o código correspondente a pessoa que você quer deletar: " );
	
	scanf("%d",&delete );
	
	if(delete<= (tamlista-1)&&delete>0){
		for(;aux != NULL&&i<= delete; aux = aux->next,i++){
			if(i == delete){
				if(aux->prev == NULL && aux->next == NULL){
					cabeca->first = NULL;
					cabeca->last = NULL;
				}else if(aux->prev == NULL){
					aux->next->prev = NULL;
					cabeca->first = aux->next;
				}else if(aux->next == NULL){
					aux->prev->next = NULL;
					cabeca->last = aux->prev;
				}else{
					aux->prev->next = aux->next;
					aux->next->prev = aux->prev;
				}
				free(aux->data);
				free(aux);
			}
		}
  }
}

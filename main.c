//gcc main.c -o mqtt_exemplo -lpaho-mqtt3c -Wall
/*Trabalho de TÓPICOS XIII CHAT MQTT PAHO

Universidade Federal da Fronteira Sul - UFFS - Campus Chapecó-SC
Estudante: Darlan Adriano Schmitz, Renan Luiz Babinski, Rodolfo Trevisol
Disciplina: Redes de Tópicos XIII
Professor: MARCO AURÉLIO SPOHN
Trabalho: SIMULAÇÃO DE UM CHAT UM PRA UM E CHAT EM GRUPO*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTClient.h"
#include "MQTTAsync.h"

#define ADDRESS     "tcp://localhost:1883"
#define CLIENTID    "ExampleClientSub"
#define TOPIC       "MQTT Examples"
#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     10000L


int geth(){                                        //PRESSIONE PARA CONTINUAR (PAUSE)
	char s;
	scanf("%c",&s);
	return 0;
}

int list_menu(){
    int menu;
    //system("clear");
    printf("MENU DO CHAT: \n\n");
    printf("1) LISTAR USUÁRIOS ONLINE \n");
    printf("2) CRIAÇÃO DE GRUPO \n");
    printf("3) VER GRUPOS CADASTRADOS\n");
    printf("4) INICIAR CONVERSA PRIVADA\n");
    printf("5) INICIAR CONVERSA EM GRUPO\n");
    printf("0) SAIR DO PROGRAMA\n");
    printf("\n\n");
    scanf("%d", &menu);
    geth();
    setbuf(stdin, NULL);
    return menu;
}

int main(int argc, char const *argv[]) {
  int menu;

  while((menu = list_menu())!= 0){
    switch (menu) {
      case 1:


        break;

      case 2:


        break;

      case 3:

        break;

      case 4:

        break;

      case 5:


        break;

      case 0:

        
        break;

      default:
        printf("Opção Inválida\n");
        geth();
        break;

   }

 }

  return 0;
}

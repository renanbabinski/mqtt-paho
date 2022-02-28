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

#define ADDRESS     "tcp://3.80.198.178:1890"
#define CLIENTID    "ExampleClientSub"
#define TOPIC       "MQTT Examples"
#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     10000L
#define DEBUG       1

//User inicialization.
int initializeUser(MQTTClient conn, MQTTClient_connectOptions opts, MQTTClient_willOptions wopts, char* userID);

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

int main(int argc, char *argv[]) {
	MQTTClient conn;
	MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
	MQTTClient_willOptions wopts = MQTTClient_willOptions_initializer;
  int menu;
  char* userID = argv[1];

  //User inicialization.
  if(initializeUser(conn, opts, wopts, userID)){

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
  } else {
    printf("An error has occured while initializing User!");
  }

  return 0;
}

int initializeUser(MQTTClient conn, MQTTClient_connectOptions opts, MQTTClient_willOptions wopts, char* userID){
	int client;
  char* userTopic = strcat(userID,"_control");

  //Initialize broker connection.
	client = MQTTClient_create(&conn, ADDRESS, userID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
	if (client != MQTTCLIENT_SUCCESS) return 0;
  
	opts.keepAliveInterval = 20;
	opts.cleansession = 1;
	opts.MQTTVersion = MQTTVERSION_DEFAULT;
	opts.username = userID;

	opts.will = &wopts;
	opts.will->message = userID;
	opts.will->qos = 1;
	opts.will->retained = 0;
	opts.will->topicName = userTopic;

	client = MQTTClient_connect(conn, &opts);
  
	if (client != MQTTCLIENT_SUCCESS) return 0;

  //userTopic subscription.
	client = MQTTClient_subscribe(conn, userTopic, 0);

  if (client != MQTTCLIENT_SUCCESS) return 0;

  //Only Debug: Publish a message to receive in terminal.
  if (DEBUG){
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken dt;
    pubmsg.payload = "Testeando";
    pubmsg.payloadlen = 9;
    pubmsg.qos = 0;
    pubmsg.retained = 0;
    client = MQTTClient_publish(conn, userTopic, pubmsg.payloadlen, pubmsg.payload, pubmsg.qos, pubmsg.retained, &dt);
  }
	
  return 1;
}

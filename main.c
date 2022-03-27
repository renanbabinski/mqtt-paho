//gcc main.c -o mqtt_exemplo -lpaho-mqtt3c -ljson-c -Wall
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
#include <json-c/json.h>

#define ADDRESS     "tcp://3.80.198.178:1890"
#define CLIENTID    "ExampleClientSub"
#define TOPIC       "MQTT Examples"
#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     10000L
#define DEBUG       1

//User initialization.
int initializeUser(MQTTClient conn, MQTTClient_connectOptions opts, MQTTClient_willOptions wopts, char* userID);
//User online status publication.
int setUserOnline(MQTTClient conn, MQTTClient_connectOptions opts, MQTTClient_willOptions wopts, char* userID);
//User offline status publication
int setUserOffline(MQTTClient conn, MQTTClient_connectOptions opts, MQTTClient_willOptions wopts, char* userID);
//List users ONLINE/OFFLINE
int listUsersStatus(MQTTClient conn, MQTTClient_connectOptions opts, MQTTClient_willOptions wopts, char* userID);
//Message arrived function
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);
//Group creation
int create_group(MQTTClient conn, MQTTClient_connectOptions opts, MQTTClient_willOptions wopts, char* userID);

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

  //Initialize broker connection.
  if (MQTTClient_create(&conn, ADDRESS, userID, MQTTCLIENT_PERSISTENCE_NONE, NULL) != MQTTCLIENT_SUCCESS){
    printf("An error has occured while initializing Client!");
    exit(EXIT_FAILURE);
  }

  //User inicialization.
  if(initializeUser(conn, opts, wopts, userID) && setUserOnline(conn, opts, wopts, userID)){

    while((menu = list_menu())!= 0){
      printf("\n\n\nMENU VALUE: %d\n", menu);
      switch (menu) {
        case 1:
          if(!listUsersStatus(conn, opts, wopts, userID)){
            printf("An error has occured while listing users status!");
            menu = 0;
          }

          break;

        case 2:
          if(!create_group(conn, opts, wopts, userID)){
            printf("An error has occured while creating group!");
            menu = 0;
          }
          geth();

          break;

        case 3:
          printf("\nOPÇÃO 3!\n");
          geth();
          break;

        case 4:
          printf("\nOPÇÃO 4!\n");
          geth();
          break;

        case 5:
          printf("\nOPÇÃO 5!\n");
          geth();
          break;

        case 0:

          
          break;

        default:
          printf("Opção Inválida\n");
          geth();
          break;

      }
    }
    if(!setUserOffline(conn, opts, wopts, userID)){
      printf("An error has occured while setting user offline!");
    }
  } else {
    printf("An error has occured while initializing User!");
  }

  return 0;
}

int initializeUser(MQTTClient conn, MQTTClient_connectOptions opts, MQTTClient_willOptions wopts, char* userID){
	int client;
  char userControlTopic[50];

  strcpy(userControlTopic, userID);
  strcat(userControlTopic,"_Control");
 
	opts.keepAliveInterval = 20;
	opts.cleansession = 1;
	opts.MQTTVersion = MQTTVERSION_DEFAULT;
	opts.username = userID;

	opts.will = &wopts;
	opts.will->message = "DISCONNECTED";
	opts.will->qos = 1;
	opts.will->retained = 0;
	opts.will->topicName = userControlTopic;

	client = MQTTClient_connect(conn, &opts);
  
	if (client != MQTTCLIENT_SUCCESS) return 0;

  //userTopic subscription.
	client = MQTTClient_subscribe(conn, userControlTopic, 0);

  if (client != MQTTCLIENT_SUCCESS) return 0;

  //Only Debug: Publish a message to receive in terminal.
  if (DEBUG){
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken dt;
    pubmsg.payload = "Testeando";
    pubmsg.payloadlen = strlen(pubmsg.payload);
    pubmsg.qos = 0;
    pubmsg.retained = 0;
    client = MQTTClient_publish(conn, userControlTopic, pubmsg.payloadlen, pubmsg.payload, pubmsg.qos, pubmsg.retained, &dt);
  }
	
  return 1;
}

int setUserOnline(MQTTClient conn, MQTTClient_connectOptions opts, MQTTClient_willOptions wopts, char* userID){
  int client;
  char userTopic[100];
  char payload[100];

  MQTTClient_message pubmsg = MQTTClient_message_initializer;
  MQTTClient_deliveryToken dt;
  sprintf(userTopic, "USERS/%s", userID);
  sprintf(payload, "{\"USER\" : \"%s\", \"STATUS\" : \"ONLINE\"}", userID);
  if(DEBUG){
    json_object *root = json_tokener_parse(payload);
    printf("The json string: \n\n%s\n\n", json_object_to_json_string(root));
    printf("The json object to string:\n\n%s\n", json_object_to_json_string_ext(root, JSON_C_TO_STRING_PRETTY));
  }
  pubmsg.payload = payload;
  pubmsg.payloadlen = strlen(pubmsg.payload);
  pubmsg.qos = 1;
  pubmsg.retained = 1;
  client = MQTTClient_publish(conn, userTopic, pubmsg.payloadlen, pubmsg.payload, pubmsg.qos, pubmsg.retained, &dt);
  
  if (client != MQTTCLIENT_SUCCESS) return 0;

  return 1;
}

int setUserOffline(MQTTClient conn, MQTTClient_connectOptions opts, MQTTClient_willOptions wopts, char* userID){
  int client;
  char userTopic[100];
  char payload[100];

  MQTTClient_message pubmsg = MQTTClient_message_initializer;
  MQTTClient_deliveryToken dt;
  sprintf(payload, "{\"USER\" : \"%s\", \"STATUS\" : \"OFFLINE\"}", userID);
  sprintf(userTopic, "USERS/%s", userID);
  if(DEBUG){
    json_object *root = json_tokener_parse(payload);
    printf("The json string: \n\n%s\n\n", json_object_to_json_string(root));
    printf("The json object to string:\n\n%s\n", json_object_to_json_string_ext(root, JSON_C_TO_STRING_PRETTY));
  }
  pubmsg.payload = payload;
  pubmsg.payloadlen = strlen(pubmsg.payload);
  pubmsg.qos = 1;
  pubmsg.retained = 1;
  client = MQTTClient_publish(conn, userTopic, pubmsg.payloadlen, pubmsg.payload, pubmsg.qos, pubmsg.retained, &dt);
  
  if (client != MQTTCLIENT_SUCCESS) return 0;

  return 1;
}

int listUsersStatus(MQTTClient conn, MQTTClient_connectOptions opts, MQTTClient_willOptions wopts, char* userID){
  MQTTClient client;
  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
  int rc;
  int ch;
  MQTTClient_create(&client, ADDRESS, CLIENTID,
      MQTTCLIENT_PERSISTENCE_NONE, NULL);
  conn_opts.keepAliveInterval = 20;
  conn_opts.cleansession = 1;
  MQTTClient_setCallbacks(client, NULL, NULL, msgarrvd, NULL);
  if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
  {
      printf("Failed to connect, return code %d\n", rc);
      exit(EXIT_FAILURE);
  }

  //userTopic subscription.
	MQTTClient_subscribe(client, "USERS/#", 0);
  // if (client != MQTTCLIENT_SUCCESS) return 0;

  do{
    ch = getchar();
  } while(ch!='Q' && ch != 'q');

  return 1;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message){
    int i;
    char* payloadptr;
    printf("topic: %s\n", topicName);
    printf("message: ");
    payloadptr = message->payload;
    for(i=0; i<message->payloadlen; i++)
    {
        putchar(*payloadptr++);
    }
    putchar('\n');
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

int create_group(MQTTClient conn, MQTTClient_connectOptions opts, MQTTClient_willOptions wopts, char* userID){
  char* group_name = "TEST_GROUP";
  int client;

  char* groupTopic = "GROUPS";
  char payload[100];

  MQTTClient_message pubmsg = MQTTClient_message_initializer;
  MQTTClient_deliveryToken dt;
  sprintf(payload, "{\"GROUP_NAME\" : \"%s\", \"OWNER\" : \"%s\", \"CREATED_TIME\" : \"2022-03-15\", \"N_MEMBERS\" : 1}", group_name, userID);
  if(DEBUG){
    json_object *root = json_tokener_parse(payload);
    printf("The json string: \n\n%s\n\n", json_object_to_json_string(root));
    printf("The json object to string:\n\n%s\n", json_object_to_json_string_ext(root, JSON_C_TO_STRING_PRETTY));
  }
  pubmsg.payload = payload;
  pubmsg.payloadlen = strlen(pubmsg.payload);
  pubmsg.qos = 1;
  pubmsg.retained = 1;
  client = MQTTClient_publish(conn, groupTopic, pubmsg.payloadlen, pubmsg.payload, pubmsg.qos, pubmsg.retained, &dt);
  
  if (client != MQTTCLIENT_SUCCESS) return 0;


  return 1;
}
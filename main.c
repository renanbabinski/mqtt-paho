//gcc main.c -o mqtt_exemplo -lpaho-mqtt3c -ljson-c -lpthread -Wall
/*Trabalho de TÓPICOS XIII CHAT MQTT PAHO

Universidade Federal da Fronteira Sul - UFFS - Campus Chapecó-SC
Estudante: Darlan Adriano Schmitz, Renan Luiz Babinski, Rodolfo Trevisol
Disciplina: Redes de Tópicos XIII
Professor: MARCO AURÉLIO SPOHN
Trabalho: SIMULAÇÃO DE UM CHAT UM PRA UM E CHAT EM GRUPO
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTClient.h"
#include "MQTTAsync.h"
#include <MQTTClientPersistence.h>
#include <json-c/json.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "lista.h"


#define ADDRESS     "tcp://3.80.198.178:1890"
#define CLIENTID    "ExampleClientSub"
#define TOPIC       "MQTT Examples"
#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     10000L
#define DEBUG       0
#define INFO        0

listHead *chatReqList;
pthread_mutex_t lock_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t printf_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t chat_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t listen_ctrl;
pthread_t chat_thread;

// GLOBALS
int user_count = 0;
char *users[50];
int chat_count = 0;
char *chats[50];

// CHAT STRUCT
typedef struct chat_context
  {
    char *topic;
    int mode;
  }chat_context;

//Payload creation.
char *createPayload(char* action, char* topic, char* source, char* message);
//User initialization.
int initializeUser(MQTTClient conn, MQTTClient_connectOptions opts, MQTTClient_willOptions wopts, char* userID);
//User online status publication.
int setUserOnline(MQTTClient conn, MQTTClient_connectOptions opts, MQTTClient_willOptions wopts, char* userID);
//User offline status publication
int setUserOffline(MQTTClient conn, MQTTClient_connectOptions opts, MQTTClient_willOptions wopts, char* userID);
//List users ONLINE/OFFLINE
int listUsersStatus(MQTTClient conn, MQTTClient_connectOptions opts, MQTTClient_willOptions wopts, char* userID, int onlyOnline);
//Message arrived function
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);
//Group creation
int create_group(MQTTClient conn, MQTTClient_connectOptions opts, MQTTClient_willOptions wopts, char* userID);
//List Each User and Status
int forEachUser(void *context, char *topicName, int topicLen, MQTTClient_message *message);
//Thread user control.
void* listen_control(void* userID);
//Test send function.
int testSend(MQTTClient conn, MQTTClient_connectOptions opts, MQTTClient_willOptions wopts, char* userID);
//Chat Thread
void* chat();
//chat arrived callback
int chat_arrived(void *context, char *topicName, int topicLen, MQTTClient_message *message);
//Join Chat
void join_chat(int mode);


int geth(){                                        //PRESSIONE PARA CONTINUAR (PAUSE)
	char s;
	scanf("%c",&s);
	return 0;
}

int list_menu(){
  int menu;
  if(!DEBUG){
    system("clear");
  }
  printf("MENU DO CHAT: \n\n");
  printf("1) LISTAR USUÁRIOS\n");
  printf("2) CRIAÇÃO DE GRUPO \n");
  printf("3) VER GRUPOS CADASTRADOS\n");
  printf("4) SOLICITAR CONVERSA PRIVADA\n");
  printf("5) SOLICITAR CONVERSA EM GRUPO\n");
  printf("6) MOSTRAR REQUISIÇÕES DE CHAT\n");
  printf("7) INGRESSAR EM CHAT PRIVADO\n");
  printf("8) INGRESSAR EM CHAT EM GRUPO\n");

  if (DEBUG){
    printf("98) TESTE ENVIAR REQUISIÇÃO\n");
    printf("99) REMOVER REQUISIÇÃO\n");
  }

  printf("0) SAIR DO PROGRAMA\n");
  printf("\n\n");
  scanf("%d", &menu);
  geth();
  setbuf(stdin, NULL);
  return menu;
}

int initializeUser(MQTTClient conn, MQTTClient_connectOptions opts, MQTTClient_willOptions wopts, char* userID){
  printf("Intializing user...\n");
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
  MQTTClient_disconnect(conn, 10000);
	
  return 1;
}

int setUserOnline(MQTTClient conn, MQTTClient_connectOptions opts, MQTTClient_willOptions wopts, char* userID){
  printf("Setting user online...\n");
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

  client = MQTTClient_connect(conn, &opts);
	if (client != MQTTCLIENT_SUCCESS){
    printf("Falha na conexão!\n");
    return 0;
  }
  

  client = MQTTClient_publish(conn, userTopic, pubmsg.payloadlen, pubmsg.payload, pubmsg.qos, pubmsg.retained, &dt);
  if (client != MQTTCLIENT_SUCCESS){
    printf("Falha na publicação!\n");
    return 0;
  } 
  MQTTClient_disconnect(conn, 10000);
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
  client = MQTTClient_connect(conn, &opts);
	if (client != MQTTCLIENT_SUCCESS) return 0;
  client = MQTTClient_publish(conn, userTopic, pubmsg.payloadlen, pubmsg.payload, pubmsg.qos, pubmsg.retained, &dt);
  
  if (client != MQTTCLIENT_SUCCESS) return 0;

  return 1;
}

int listUsersStatus(MQTTClient conn, MQTTClient_connectOptions opts, MQTTClient_willOptions wopts, char* userID, int onlyOnline){
  MQTTClient client;
  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
  int rc;
  int ch;
  MQTTClient_create(&client, ADDRESS, userID,
      MQTTCLIENT_PERSISTENCE_NONE, NULL);
  conn_opts.keepAliveInterval = 20;
  conn_opts.cleansession = 1;
  MQTTClient_setCallbacks(client, &onlyOnline, NULL, forEachUser, NULL);
  if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
  {
      printf("Failed to connect, return code %d\n", rc);
      exit(EXIT_FAILURE);
  }

  //userTopic subscription.
  if(!onlyOnline){
    printf("Pressione ENTER para sair...\n\n");
  }else{
    printf("Pressione ENTER para continuar e selecionar um usuário...\n\n");
    user_count = 0;
  }


	MQTTClient_subscribe(client, "USERS/#", 0);
  // if (client != MQTTCLIENT_SUCCESS) return 0;
  do{
    ch = getchar();
  } while(ch!='\n');

  MQTTClient_disconnect(client, 10000);
  MQTTClient_destroy(&client);

  return 1;
}

int forEachUser(void *context, char *topicName, int topicLen, MQTTClient_message *message){
  int online_mode = *(int *)context;
  json_object *root = json_tokener_parse((char *)message->payload);
  json_object *user = json_object_object_get(root, "USER");
  json_object *status = json_object_object_get(root, "STATUS");

  if(online_mode){
    if(!strcmp(json_object_get_string(status), "ONLINE")){
      char *user_name = json_object_get_string(user);
      users[user_count] = user_name;
      printf("%d) USUÁRIO: %s    STATUS: %s\n", user_count, json_object_get_string(user), json_object_get_string(status));
      user_count++;
    }
  }else{
    printf("USUÁRIO: %s    STATUS: %s\n", json_object_get_string(user), json_object_get_string(status));
  }
  
  MQTTClient_freeMessage(&message);
  MQTTClient_free(topicName);
  return 1;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message){
    json_object *root, *action, *source, *timeStamp, *payload;
    char* payloadptr = message->payload;
    
    //Locks mutex to use exclusively the shared lists.
    pthread_mutex_lock(&lock_mutex);
    
      root = json_tokener_parse(payloadptr);
      action = json_object_object_get( root, "ACTION");
      payload = json_object_object_get( root, "PAYLOAD");

      if(DEBUG){
        json_object *source = json_object_object_get( root, "SOURCE");
        printf("\nACTION RECEIVED FROM SOURCE %s: %s\n", json_object_get_string(source), json_object_get_string(action));
      }
      
      char *actionStr = (char *)json_object_get_string(action);
      
      //Action cases:
      if (strcmp(actionStr,"CHATREQ") == 0){ //Chat request.
        source = json_object_object_get( root, "SOURCE");
        timeStamp = json_object_object_get( root, "TIMESTAMP");
        
        //Create a chat request.
        request *chatReq = malloc(sizeof(request));
        chatReq->payload = (char *)json_object_get_string(payload);
        chatReq->source = (char *)json_object_get_string(source);
        chatReq->timeStamp = (char *)json_object_get_string(timeStamp);

        //Includes request in the list.
        insertReq(chatReqList, chatReq);
        
        if(DEBUG){
          printf("\n--------------------------------------------------");
          printf("\npayload recebido: %s", chatReq->payload);
          printf("\n--------------------------------------------------");
          printReqs(chatReqList);
        }
      }else if(strcmp(actionStr,"ACKCHAT") == 0){ //Accepted chat request
        source = json_object_object_get( root, "SOURCE");
        timeStamp = json_object_object_get( root, "TIMESTAMP");

        printf("\n--------------------------------------------------");
        printf("\n%s Aceitou sua solicitação!\nIngresse na conversa pelo tópico %s no MENU", json_object_get_string(source), json_object_get_string(payload));
        printf("\n--------------------------------------------------\n");

        char *chat_topic = json_object_get_string(payload);
        chats[chat_count] = chat_topic;
        chat_count++;

      }

      //MQTTClient_freeMessage(&message);
      //MQTTClient_free(topicName);
      
    //Locks mutex for further use.
    pthread_mutex_unlock(&lock_mutex);
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
  client = MQTTClient_connect(conn, &opts);
	if (client != MQTTCLIENT_SUCCESS) return 0;
  client = MQTTClient_publish(conn, groupTopic, pubmsg.payloadlen, pubmsg.payload, pubmsg.qos, pubmsg.retained, &dt);
  
  if (client != MQTTCLIENT_SUCCESS) return 0;

  MQTTClient_disconnect(conn, 10000);
  return 1;
}

int request_chat(MQTTClient conn, MQTTClient_connectOptions opts, MQTTClient_willOptions wopts, char* userID){
  int user_index;
  char user_msg[200];

  if(!listUsersStatus(conn, opts, wopts, userID, 1)){
    printf("An error has occured while listing users status");
    exit(EXIT_FAILURE);
  }
  printf("Selecione o número do usuário:\n");
  scanf("%d", &user_index);
  geth();
  

  printf("Você selecionou: %s\n", users[user_index]);
  printf("Escreva uma mensagem de solicitação para o usuário %s\n", users[user_index]);

  fgets(user_msg, sizeof(user_msg), stdin);

  // Removing new line at the end
  user_msg[strcspn(user_msg, "\n")] = 0;
  

  printf("Sua mensagem:\n%s\n", user_msg);

  char user_control_topic[50];

  strcpy(user_control_topic, users[user_index]);
  strcat(user_control_topic,"_Control");

  printf("Enviando solicitação no tópico de controle %s ...\n", user_control_topic);


  int client;
  char *action = "CHATREQ";
  char *topic = user_control_topic;
  char *source = userID;
  char *message = user_msg;        
  char *jsonRet = createPayload(action, topic, source, message);

  MQTTClient_message pubmsg = MQTTClient_message_initializer;
  MQTTClient_deliveryToken dt;
 
  pubmsg.payload = jsonRet;
  pubmsg.payloadlen = strlen(pubmsg.payload);
  pubmsg.qos = 1;
  pubmsg.retained = 0;
  client = MQTTClient_connect(conn, &opts);
	if (client != MQTTCLIENT_SUCCESS) return 0;

  client = MQTTClient_publish(conn, topic, pubmsg.payloadlen, pubmsg.payload, pubmsg.qos, pubmsg.retained, &dt);
  if (client != MQTTCLIENT_SUCCESS) return 0;

  printf("Solicitação Enviada!");

  return 1;
}


char *createPayload(char* action, char* topic, char* source, char* message){
  time_t timeStamp;
  time(&timeStamp);

  char json[1000];

  sprintf(json, "{\"ACTION\" : \"%s\", \"TOPIC\" : \"%s\", \"TIMESTAMP\" : \"%ld\", \"SOURCE\" : \"%s\", \"PAYLOAD\" : \"%s\" }", action, topic, timeStamp, source, message);
  json_object *root = json_tokener_parse(json);

  if(DEBUG){
    //printf("The json string: \n\n%s\n\n", json_object_to_json_string(root));
    printf("The json object to string:\n\n%s\n", json_object_to_json_string_ext(root, JSON_C_TO_STRING_PRETTY));
  }

  char *payload = malloc(strlen(json)*sizeof(char));

  memcpy(payload, json, strlen(json)+1);

  return payload;
}


// THREAD ->>> CONTROL MESSAGES
void* listen_control(void* userID){
  char topic[50];
  strcpy(topic, userID);
  strcat(topic,"_Control");

  char id[100];

  sprintf(id, "listen_control_id_%s", topic);

  if(DEBUG){
		pthread_mutex_lock(&printf_mutex);
    printf("Iniciando a Thread de controle do tópico %s\n", topic);
		pthread_mutex_unlock(&printf_mutex);
  }

  MQTTClient client;
  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
  
  MQTTClient_create(&client, ADDRESS, id, MQTTCLIENT_PERSISTENCE_NONE, NULL);
  
  conn_opts.keepAliveInterval = 20;
  conn_opts.cleansession = 1;
  MQTTClient_setCallbacks(client, NULL, NULL, msgarrvd, NULL);

  int rc;
  if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS){
      printf("Failed to connect, return code %d\n", rc);
      exit(EXIT_FAILURE);
  }

  MQTTClient_subscribe(client, topic, QOS);
  return 0;
}


int accept_chat(MQTTClient conn, MQTTClient_connectOptions opts, MQTTClient_willOptions wopts, char* userID){
  int accepted_req;
  char *user_name;
  time_t timeStamp;
  time(&timeStamp);
  char user_control_topic[50];
  char* chat_topic = malloc(sizeof(char)* 200);
  char *timestamp = malloc(200 * sizeof(char));
  sprintf(timestamp, "%ld", timeStamp);

  printf("Se deseja aceitar uma solicitação, digite o número correspondente:\nOu digite 0 para sair...\n");
  scanf("%d", &accepted_req);
  

  if(accepted_req > 0){

    user_name = popReqUserName(chatReqList, accepted_req);

    strcpy(user_control_topic, user_name);
    strcat(user_control_topic,"_Control");


    //Defined Topic name. Format: X_Y_TIMESTAMP
    strcpy(chat_topic, user_name);
    strcat(chat_topic, "_");
    strcat(chat_topic, userID);
    strcat(chat_topic, "_");
    strcat(chat_topic, timestamp);

    printf("\n--------------------------------------------------");
    printf("\nVocê aceitou a solicitação para conversar com %s!\nIngresse na conversa pelo tópico %s no MENU", user_name, chat_topic);
    printf("\n--------------------------------------------------\n");

    char *topic_aux = malloc(sizeof(char)*200);
    sprintf(topic_aux, "%s", chat_topic);
    chats[chat_count] = topic_aux;
    chat_count++;

    int client;
    char *action = "ACKCHAT";
    char *topic = user_control_topic;
    char *source = userID;
    char *message = chat_topic;        
    char *jsonRet = createPayload(action, topic, source, message);

    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken dt;
  
    pubmsg.payload = jsonRet;
    pubmsg.payloadlen = strlen(pubmsg.payload);
    pubmsg.qos = 1;
    pubmsg.retained = 0;
    client = MQTTClient_connect(conn, &opts);
    if (client != MQTTCLIENT_SUCCESS) return 0;

    client = MQTTClient_publish(conn, topic, pubmsg.payloadlen, pubmsg.payload, pubmsg.qos, pubmsg.retained, &dt);
    if (client != MQTTCLIENT_SUCCESS) return 0;

    join_chat(1);
  }
  
  geth();
  
  return 1;
}


// THREAD ->>> CHAT
void* chat(void* chat_struct){
  system("clear");
  chat_context *chat_c = (chat_context*)chat_struct;

  char keyword[200];
  char id[100];
  char payload[300];
  char *chat_topic = chat_c->topic;
  char *temp_topic = malloc(sizeof(char)*strlen(chat_topic));
  char *temp;
  
  sprintf(temp_topic, "%s", chat_topic);

  temp = strtok(temp_topic, "_");
  char *my_user_name = temp;
  temp = strtok(NULL, "_");
  char *dst_user = temp;
  temp = strtok(NULL, "_");
  char *timestamp = temp;
  
  if(chat_c->mode == 0){
    sprintf(id, "listen_chat_id_%s", my_user_name);
  }else{
    sprintf(id, "listen_chat_id_%s", dst_user);
  }

  if(chat_c->mode == 0){
    printf("Você iniciou um chat no tópico %s com o usuário %s\nDigite !quit a qualquer momento para sair do chat!\n", chat_topic, dst_user);
  }else{
    printf("Você iniciou um chat no tópico %s com o usuário %s\nDigite !quit a qualquer momento para sair do chat!\n", chat_topic, my_user_name);
  }


  MQTTClient client;
  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
  
  printf("Creating MQTTClient...\n");
  MQTTClient_create(&client, ADDRESS, id, MQTTCLIENT_PERSISTENCE_NONE, NULL);
  
  conn_opts.keepAliveInterval = 20;
  conn_opts.cleansession = 1;
  printf("Setting callback...\n");
  MQTTClient_setCallbacks(client, NULL, NULL, chat_arrived, NULL);

  int rc;
  printf("Connection...\n");
  if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS){
      printf("Failed to connect, return code %d\n", rc);
      exit(EXIT_FAILURE);
  }

  printf("Subscribing...\n");
  MQTTClient_subscribe(client, chat_topic, QOS);


  while(1){
    fgets(keyword, sizeof(keyword), stdin);
    keyword[strcspn(keyword, "\n")] = 0;
    if(chat_c->mode == 0){
      sprintf(payload, "%s: %s", my_user_name, keyword);
    }else{
      sprintf(payload, "%s: %s", dst_user, keyword);
    }
    if(strcmp(keyword, "!quit") != 0){
      MQTTClient_message pubmsg = MQTTClient_message_initializer;
      MQTTClient_deliveryToken dt;
      pubmsg.payload = payload;
      pubmsg.payloadlen = strlen(pubmsg.payload);
      pubmsg.qos = 2;
      pubmsg.retained = 0;
      MQTTClient_publish(client, chat_topic, pubmsg.payloadlen, pubmsg.payload, pubmsg.qos, pubmsg.retained, &dt);
    }else{
      break;
    }
  }

  printf("Você saiu do chat! Pressione ENTER para voltar ao menu...");

  MQTTClient_disconnect(client, 10000);
  MQTTClient_destroy(&client);

  pthread_exit(NULL);
}


void join_chat(int mode){
  chat_context *chat_c = malloc(sizeof(chat_context));
  chat_c->mode = mode;
  
  int i = 1;
  int chat_index;
  printf("Chats Disponíveis:\n");
  while(chats[i-1] != NULL){
    printf("%d) Chat topic: %s\n", i, chats[i-1]);
    i++;
  }
  printf("Selecione o número do chat para ingressar:\n");
  scanf("%d", &chat_index);
  geth();

  printf("Você escolheu: %s\nPressione ENTER para ingressar no chat...", chats[chat_index-1]);

  chat_c->topic = chats[chat_index-1];

  pthread_create(&chat_thread, NULL, chat, chat_c);
  printf("\nThread: %ld\n", chat_thread);
  pthread_join(chat_thread, NULL);


}

int chat_arrived(void *context, char *topicName, int topicLen, MQTTClient_message *message){
    int i;
    char* payloadptr;
    // printf("You:");
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




////////////////////////// MAIN //////////////////////////////

int main(int argc, char *argv[]) {
	MQTTClient conn;
	MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
	MQTTClient_willOptions wopts = MQTTClient_willOptions_initializer;
  chatReqList = malloc(sizeof(listHead));
  chatReqList->listSize = 0;
  
  int menu;
  char* userID = argv[1];

  char controlTopic[50];

  strcpy(controlTopic, userID);
  strcat(controlTopic,"_Control");
  

  //Initialize broker connection.
  if (MQTTClient_create(&conn, ADDRESS, userID, MQTTCLIENT_PERSISTENCE_NONE, NULL) != MQTTCLIENT_SUCCESS){
    printf("An error has occured while initializing Client!");
    exit(EXIT_FAILURE);
  }

  // sleep(1);

  //User inicialization.
  if(initializeUser(conn, opts, wopts, userID) && setUserOnline(conn, opts, wopts, userID)){

    pthread_create(&listen_ctrl, NULL, listen_control, userID);
    printf("\nThread: %ld\n", listen_ctrl);
    sleep(2);

    while((menu = list_menu())!= 0){
      printf("\n\n\nMENU VALUE: %d\n", menu);
      switch (menu) {
        case 1:
          system("clear");
          if(!listUsersStatus(conn, opts, wopts, userID, 0)){
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
          system("clear");
          if(!request_chat(conn, opts, wopts, userID)){
            printf("An error has occured while requesting chat!");
            menu = 0;
          }
          geth();
          break;

        case 5:
          printf("\nOPÇÃO 5!\n");
          geth();
          break;

        case 6:
          system("clear");
	        printf("\n\n|----------LIST OF REQUESTED CHATS----------|");
          printReqs(chatReqList);
	        printf("\n\n|-------------------------------------------|\n");
          accept_chat(conn, opts, wopts, userID);
	        printf("\n\n");
          geth();
          break;

        case 7:
          system("clear");
          join_chat(0);
          geth();
          break;

        case 8:
          printf("\nOPÇÃO 8!\n");
          geth();
          break;

        case 98:
          // printf("\nOPÇÃO 7!\n");
          testSend(conn, opts, wopts, controlTopic);
          break;

        case 99:
          // printf("\nOPÇÃO 8!\n");
          removeReq(chatReqList, 1);
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





int testSend(MQTTClient conn, MQTTClient_connectOptions opts, MQTTClient_willOptions wopts, char* userID){
  int client;
  char *action = "CHATREQ";
  char *topic = userID;
  char *source = "EU TESTANDO";
  char *message = "ISSO É UM TESTE PRA VER A LISTA";        
  char *jsonRet = createPayload(action, topic, source, message);

  MQTTClient_message pubmsg = MQTTClient_message_initializer;
  MQTTClient_deliveryToken dt;
 
  pubmsg.payload = jsonRet;
  pubmsg.payloadlen = strlen(pubmsg.payload);
  pubmsg.qos = 1;
  pubmsg.retained = 0;
  client = MQTTClient_connect(conn, &opts);
	if (client != MQTTCLIENT_SUCCESS) return 0;

  client = MQTTClient_publish(conn, topic, pubmsg.payloadlen, pubmsg.payload, pubmsg.qos, pubmsg.retained, &dt);
  if (client != MQTTCLIENT_SUCCESS) return 0;

  return 1;
}
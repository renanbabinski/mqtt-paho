# MQTT CHAT #  

Aplicação de chat implementada sobre o protocolo MQTT.

## Alunos ##

* Renan Babinski
* Rodolfo Trevisol
* Darlan Schmitz

## Dependencias: ##  

### Instalação dos pacotes de desenvolvimento do OpenSSL:
* A bibloteca Paho MQTT da suporte à comunicação segura com MQTT (utilizando TLS/SSL), logo os pacotes de desenvolvimento do OpenSSL são necessários para sua compilação e funcionamento. Para instalar os pacotes de desenvolvimento do OpenSSL, execute os comandos a seguir:

        sudo apt-get install libssl-dev

### Instalação da biblioteca Paho (Client side):

* Linux:  

        git clone https://github.com/eclipse/paho.mqtt.c.git
        cd org.eclipse.paho.mqtt.c.git
        make

    To install:

        sudo make install

### Instalação da biblioteca JSON-C:

* Linux:

        sudo apt install libjson-c-dev


## Funcionalidades:

- Usuário cria e assina seu próprio tópico de controle

- Usuário manda mensagem no tópico USERS/{UserID} sempre que fica online e essa mensagem é retida

- Usuário manda mensagem no tópico USERS/{UserID} sempre que fica offline e essa mensagem é retida

- É possivel listar os status de todos os usuários (Sem formatação, apenas JSON como foi publicado)

## Funções:

### Montagem do Payload:
* Declaração dos valores para função:
```
  char* action = "STRING";
  char* topic = "STRING";
  char* source = "STRING";
  char* message = "STRING";
  int payloadSize = strlen(action) +
                    strlen(topic) +
                    strlen(source) +
                    strlen(message);          
  char* jsonRet = malloc(payloadSize);
```
* Chamada da função:
```
   createPayload(jsonRet, payloadSize, action, topic, source, message);
```
* Confirmação do retorno:
```
  printf("\njson retornado:\n %s", jsonRet);
```
* Lembre-se de liberar a memória:
```
  free(jsonRet);
```
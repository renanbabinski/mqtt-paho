# MQTT CHAT #  

Aplicação de chat implementada sobre o protocolo MQTT.

## Dependencias: ##  

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
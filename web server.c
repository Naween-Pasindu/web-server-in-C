#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<winsock2.h>
#include<windows.h>
#include <ws2tcpip.h>
#include <dirent.h>
#include <errno.h>

#pragma comment(lib, "ws2_32.lib")

#define buffer_size 1024
#define root "www"
#define port 8080

//global declaration
FILE *fptr;
DIR *dir;

int netSocket,portNo = port;
char *path,*extension,request[100]={'\0'};
socklen_t addrlen;

struct clientList {
    SOCKET socket;
    socklen_t length;
    struct sockaddr_storage address;
    struct clientList *next;
    struct clientList *previous;
};

static struct clientList *clients = NULL;

void sendFile(struct clientList *);
void parse(char*);
void contentType(int);
void validate(int);
struct clientList *addClient();
void removeClient(struct clientList*);

int main() {
    char *buffer = malloc(buffer_size);
    char address_buffer[40];
    WORD versionWanted = MAKEWORD(1,1);
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0){
        printf("Failed. Error Code : %d.\nPress a key to exit...", WSAGetLastError());
        return 1;
    }
    //create socket
    netSocket = socket(AF_INET,SOCK_STREAM,0);
    printf("Configuring server...\n");
    //specify an address for the socket
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(portNo);

    //binding
    addrlen = sizeof((struct SOCKADDR *)&address);
    bind(netSocket,(struct SOCKADDR *)&address,(socklen_t *) &addrlen);
    if (listen(netSocket,20) < 0) {
        printf(stderr, "listening failed...\n");
        exit(EXIT_FAILURE);
    }
    while(1){
        fd_set reads;
        FD_ZERO(&reads);
        FD_SET(netSocket,&reads);
        SOCKET max_socket = netSocket;
        struct clientList *client = clients;

        while(client) {
            FD_SET(client->socket, &reads);
            if (client->socket > max_socket){
                max_socket = client->socket;
            }
            client = client->next;
        }
        printf("Listening...\n");
        if(select(max_socket+1,&reads,NULL,NULL,NULL)<0){
            printf("SELECT error.\n");
            exit(EXIT_FAILURE);
        }
        if (FD_ISSET(netSocket, &reads)) {
            client = addClient(NULL);
            client->socket = accept(netSocket,(struct sockaddr*) &(client->address),&(client->length));
            getnameinfo((struct sockaddr*)&client->address,client->length,address_buffer, sizeof(address_buffer), 0, 0,NI_NUMERICHOST);
            printf("Incomming connection from: %s\n",address_buffer);
            printf("Assigned port :%d\n",client->socket);
        }

        client = clients;
        while(client) {
                if (FD_ISSET(client->socket, &reads)) {
                    int r = recv(client->socket,buffer, buffer_size,0);
                    if (r < 1) {
                        removeClient(client);
                    } else {
                        getnameinfo((struct sockaddr*)&client->address,client->length,address_buffer, sizeof(address_buffer), 0, 0,NI_NUMERICHOST);
                        printf("%s:%d requesting : ",address_buffer,client->socket);
                        parse(buffer);
                        validate(client->socket);
                        sendFile(client);
                        printf("File send to : %s\n",address_buffer);
                        printf("socket closed...\n");
                        memset(buffer, '\0',buffer_size);
                        FD_CLR(client->socket,&reads);
                    }
                    closesocket(client->socket);
                }
                client = client->next;
        }
    }
    WSACleanup();
    closesocket(netSocket);
    return EXIT_SUCCESS;
}

void sendFile(struct clientList *client){
    char temp[25]={'\0'},c;
    fseek(fptr, 0L, SEEK_END);
    size_t size = ftell(fptr);
    rewind(fptr);
    send(client->socket,"Connection: close\n",18,0);
    sprintf(temp,"Content-length: %d\n",size);
    send(client->socket,temp,strlen(temp),0);
    contentType(client->socket);
    char *string = malloc(sizeof(char)*(size));
    memset(string, '\0',size);
    fread(string, 1, size, fptr);
    send(client->socket,string,size, 0);
    fclose(fptr);
    memset(request, '\0',100);
    removeClient(client);
}

void parse(char* line){
    path = strtok(line, "\n"); // get the first line of http request
    printf("%s\n",path);
    path = strtok(path, " ");
    path = strtok(NULL, " ");
}

void contentType(int clientPort){
      char type[70] = {'\0'};
      extension = strrchr(path,'.');// This returns a pointer to the first occurrence of some character in the string
      if((strcmp(extension,".htm"))==0 || (strcmp(extension,".html"))==0){
            strcpy(type,"Content-Type: text/html\n\n");
      }else if((strcmp(extension,".jpg"))==0){
            strcpy(type,"Content-Type: image/jpeg\n\n");
      }else if(strcmp(extension,".gif")==0){
            strcpy(type,"Content-Type: image/gif\n\n");
      }else if(strcmp(extension,".txt")==0){
            strcpy(type,"Content-Type: text/plain\n\n");
      }else if(strcmp(extension,".js")==0){
            strcpy(type,"Content-Type: application/x-javascript\n\n");
      }else if(strcmp(extension,".css")==0){
            strcpy(type,"Content-Type: text/css\n\n");
      }else if(strcmp(extension,".png")==0){
            strcpy(type,"Content-Type: image/png\n\n");
      }else if(strcmp(extension,".mp4")==0){
            strcpy(type,"Content-Type: video/mp4\n\n");
      }else{
        printf("its folder\n");
      }
      send(clientPort,type,strlen(type),0);
}

void validate(int clientPort){
    extension = strrchr(path,'.');

    if(extension == NULL){
            char temp[50] = {'\0'};
            strcpy(temp,root);
            strcat(temp,path);
            strcpy(path,temp);
            dir = opendir(path);
            if (dir) {
                int a;
                a = strlen(path)-1;
                if(path[a]=='/'){
                    strcat(path, "index.html");
                }else{
                    strcat(path, "/index.html");
                }
                if ((fptr = fopen(path,"rb")) == NULL){
                    send(clientPort, "HTTP/1.1 404 NOT FOUND\n", 23,0);
                    printf("Response code :404\n");
                    strcpy(path, "assets/404.html");
                }else{
                    send(clientPort, "HTTP/1.1 200 OK\n", 16,0);
                    printf("Response code :200\n");
                }
            } else if (ENOENT == errno) {
                send(clientPort, "HTTP/1.1 404 NOT FOUND\n", 23,0);
                printf("Response code :404\n");
                strcpy(path, "assets/404.html");
                fptr = fopen(path,"rb");
            } else {
                send(clientPort, "HTTP/1.1 500 INTERNAL SERVER ERROR\n", 35,0);
                printf("Response code :500\n");
                strcpy(path, "assets/500.html");
                fptr = fopen(path,"rb");
            }

            closedir(dir);
    }else{
            strcpy(request,root);
            strcat(request,path);
            if ((fptr = fopen(request,"rb")) == NULL){
                send(clientPort, "HTTP/1.1 404 NOT FOUND\n", 23,0);
                strcpy(path, "assets/404.html");
                printf("Response code :404\n");
                fptr = fopen(path,"r");
            }else{
                send(clientPort, "HTTP/1.1 200 OK\n", 16,0);
                printf("Response code :200\n");
            }
    }
}

struct clientList *addClient(){
    struct clientList *new_node = (struct clientList*)malloc(sizeof(struct clientList));
    struct clientList *last = clients;
    new_node->next = NULL;
    if (clients == NULL) {
        new_node->previous = NULL;
        clients = new_node;
        return new_node;
    }
    while (last->next != NULL)
        last = last->next;
    last->next = new_node;
    new_node->previous = last;
    return new_node;
}

void removeClient(struct clientList *del){
    if (clients == NULL || del == NULL)
        return;
    if (clients == del)
        clients = del->next;
    if (del->next != NULL)
        del->next->previous = del->previous;
    if (del->previous != NULL)
        del->previous->next = del->next;
    free(del);
}

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

#define buffer_size 2048
#define root "www"
#define port 5000

//global declaration
FILE *fptr;
DIR *dir;

int netSocket,portNo = port,client;
char *path,*extension,request[100]={'\0'};
socklen_t addrlen;

int main() {
    char c = 0,*buffer = malloc(buffer_size);
    WORD versionWanted = MAKEWORD(1,1);
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0){
        printf("Failed. Error Code : %d.\nPress a key to exit...", WSAGetLastError());
        c = getch();
        return 1;
    }
    //create socket
    netSocket = socket(AF_INET,SOCK_STREAM,0);

    //specify an address for the socket
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(portNo);

    //binding
    addrlen = sizeof((struct SOCKADDR *)&address);
    bind(netSocket,(struct SOCKADDR *)&address,(socklen_t *) &addrlen);
    listen(netSocket,20);
    addrlen = sizeof(address);
    int receive;
    while(1){
        printf("open a new socket...\n");
        client = accept(netSocket,NULL,NULL);
        printf("waiting connection...\n");
        //client = accept(netSocket,(struct sockaddr *) &address, &addrlen);
        receive = recv(client, buffer, buffer_size,0); //wait until receive the complete request
        if(receive==0){
            continue;
        }
        if(strlen(buffer)<10){
            printf("jump to next\n");
            continue;
        }
        parse(buffer);
        validate();
        sendFile(client);
        //send(client, "HTTP/1.1 200 OK\n", 16,0);
        //send(client, "Content-length: 46\n", 19,0);
        //send(client, "Content-Type: text/html\n\n", 25,0);
        //send(client, "<html><body><H1>Hello world</H1></body></html>",46,0);
        printf("File sent\n");
        close(client);
        printf("socket closed...\n");
    }
    close(netSocket);
    return 0;
}

void sendFile(int client){
    char temp[25]={'\0'},c;
    long size =0;
    while((c=fgetc(fptr))) {
        if(c == EOF) break;
        size++;
    }
    //fseek(fptr, 0, SEEK_END);
    //size = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);
    sprintf(temp,"Content-length: %d\n",size);
    send(client,temp,strlen(temp),0);
    contentType();
    char *string = malloc(sizeof(char)*(size));
    memset(string, '\0',size);
    fread(string, 1, size, fptr);
    send(client,string,size, 0);
    fclose(fptr);
    memset(request, '\0',100);
}

void parse(char* line){
    path = strtok(line, "\n"); // get the first line of http request
    path = strtok(path, " ");
    path = strtok(NULL, " ");
}

void contentType(){
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
      }else{
        printf("its folder\n");
      }
      send(client,type,strlen(type),0);
}

void validate(){
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
                printf("path - %s\n",path);
                if(path[a]=='/'){
                    strcat(path, "index.html");printf("path is: %s\n",path);
                }else{
                    strcat(path, "/index.html");printf("path is: %s\n",path);
                }
printf("path - %s\n",path);
                if ((fptr = fopen(path,"r")) == NULL){
                    send(client, "HTTP/1.1 404 OK\n", 16,0);
                    strcpy(path, "assets/404.html");
                }else{
                    send(client, "HTTP/1.1 200 OK\n", 16,0);
                }
            } else if (ENOENT == errno) {
                send(client, "HTTP/1.1 404 OK\n", 16,0);
                strcpy(path, "assets/404.html");
            } else {
                send(client, "HTTP/1.1 500 OK\n", 16,0);
                strcpy(path, "assets/500.html");
            }
            fptr = fopen(path,"r");
            closedir(dir);
            printf("file - %s\n",path);
    }else{
            strcpy(request,root);
            strcat(request,path);
            if ((fptr = fopen(request,"r")) == NULL){
                send(client, "HTTP/1.1 404 OK\n", 16,0);
                strcpy(path, "assets/404.html");
                fptr = fopen(path,"r");
                printf("file - %s\n",path);
            }else{
                send(client, "HTTP/1.1 200 OK\n", 16,0);
                printf("file - %s\n",request);
            }
    }
}

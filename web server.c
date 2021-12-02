#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<winsock2.h>
#include<windows.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#define buffer_size 1024

//global declaration
FILE *fptr;


int main() {
    char c = 0;
    WORD versionWanted = MAKEWORD(1,1);
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0){
        printf("Failed. Error Code : %d.\nPress a key to exit...", WSAGetLastError());
        c = getch();
        return 1;
    }

    // create socket
    int netSocket,portNo = 5000,client;
    netSocket = socket(AF_INET,SOCK_STREAM,0);
    socklen_t addrlen;

    //specify an address for the socket
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(portNo);
    printf("connection failed\n");

    //binding
    addrlen = sizeof((struct SOCKADDR *)&address);
    bind(netSocket,(struct SOCKADDR *)&address, &addrlen);
    listen(netSocket,10);
    printf("connection failed\n");
    char messege[20] = "<h1>Hello from server</h1>";
    addrlen = sizeof(address);


    while(1){
        client = accept(netSocket,(struct sockaddr *) &address, &addrlen);
    //send(client,messege,sizeof(messege),0);
          send(client, "HTTP/1.1 200 OK\n", 16,0);
      send(client, "Content-length: 46\n", 19,0);
      send(client, "Content-Type: text/html\n\n", 25,0);
      send(client, "<html><body><H1>Hello world</H1></body></html>",46,0);
    close(netSocket);
    }

    return 0;
}

void readFile(int &socket){

}

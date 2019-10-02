#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "structs.h"
#include "functions.h"

void perror_exit(char *message){
    perror(message);
    exit(EXIT_FAILURE);
}

int numLength(int num){
    if (num < 10)
        return 1;
    return 1 + numLength(num / 10);
}

void connect_request(int *sockfd, struct sockaddr_in *my_addr, int port, char* ipAddr)
{
  	int yes = 1;

  	if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    		perror("Socket");
    		exit(1);
  	}

    struct hostent *rem;

    //vriskw to serverIP
    if ((rem = gethostbyname(ipAddr)) == NULL){
        perror_exit("gethostbyname");
    }
  	my_addr->sin_family = AF_INET;
  	my_addr->sin_port = htons(port);
  	memcpy(&my_addr->sin_addr, rem->h_addr, rem->h_length);

  	if (setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    		perror("setsockopt");
    		exit(1);
  	}

  	if (bind(*sockfd, (struct sockaddr *)my_addr, sizeof(struct sockaddr)) == -1) {
    		perror_exit("Unable to bind");
  	}
  	if (listen(*sockfd, 10) == -1) {
    		perror_exit("listen");
  	}
  	printf("\nClient Waiting for server on port %d\n", port);
}

void connection_accept(fd_set *master, int *fdmax, int sockfd, struct sockaddr_in *client_addr)
{
  	socklen_t addrlen;
  	int newsockfd;

  	addrlen = sizeof(struct sockaddr_in);
  	if((newsockfd = accept(sockfd, (struct sockaddr *)client_addr, &addrlen)) == -1) {
    		perror_exit("accept");
  	}
    else {
    		FD_SET(newsockfd, master);
    		if(newsockfd > *fdmax){
    	       *fdmax = newsockfd;
    		}
    		printf("new connection from on port \n");
  	}
}

void send_client_to_clients(int portNum, clientListNode* head, char* ipAddr, int cl_portNum){
    int cl_sockfd;
    clientListNode* tmp = head;

    char* user_on = malloc(strlen("USER_ON <IP  , >") + strlen(ipAddr) + numLength(cl_portNum) + 1);

    sprintf(user_on, "USER_ON <IP %s , %d>", ipAddr, cl_portNum);

    while(tmp != NULL){
        if((tmp->portNum == cl_portNum) && (strcmp(tmp->ipAddr, ipAddr) == 0)){ //an einai o idios client pou ekane log_on mh steileis kati
            tmp = tmp->next;
            continue;
        }

        cl_sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (cl_sockfd == -1) {
            perror_exit("socket");
        }

        struct sockaddr_in client;
        struct hostent *rem;

        //vriskw to serverIP
        if ((rem = gethostbyname(tmp->ipAddr)) == NULL){
            perror_exit("gethostbyname");
        }
        //assign IP, PORT
        client.sin_family = AF_INET;
        memcpy(&client.sin_addr, rem->h_addr, rem->h_length);
        client.sin_port = htons(tmp->portNum);

        //connect the server socket to client socket
        if (connect(cl_sockfd, (struct sockaddr *)&client, sizeof(client)) != 0) {
            perror_exit("connect");
        }
        else
            printf("connected to the client with ip %s and sending %d\n", ipAddr, user_on);

        send(cl_sockfd, user_on, strlen(user_on)+1, 0);

        tmp = tmp->next;
        close(cl_sockfd);
    }
    free(user_on);
    return;
}

void send_clients(clientListNode* head, int fd){
    clientListNode* curr = head;
    int n = numOfNodes(head);
    printClientList(head);

    char* client_list = malloc(strlen("CLIENT_LIST  ") + numLength(n) + 1);
    sprintf(client_list, "CLIENT_LIST %d ", n);
    send(fd, client_list, strlen(client_list)+1, 0);

    while(curr != NULL){
        char* ip_port = malloc(strlen("<  ,  >") + strlen(curr->ipAddr) + numLength(curr->portNum) + 1);
        sprintf(ip_port, "< %s , %d >", curr->ipAddr, curr->portNum);
        sleep(1); //gia na ginoun ta read alliws kollaei
        write(fd, ip_port, strlen(ip_port)+1);
        curr = curr->next;
        memset(ip_port, '0', sizeof(ip_port));
        free(ip_port);
    }
    //send(fd, "ip_port", strlen("ip_port")+1, 0);
    free(client_list);
    return;
}

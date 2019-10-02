#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <netdb.h>
#include <signal.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "structs.h"
#include "functions.h"

static int flag = 0;

void sigintHandler(int sig_num)
{
    signal(SIGINT, sigintHandler);
    printf("\n INTERAAAPT \n");
    flag = 1;
    fflush(stdout);
    return;
}

int main(int argc,char* argv[]){

    int arg;
    char* dirName, *serverIP;
    int portNum, workerThreads, bufferSize, serverPort;

    dirName = malloc(strlen(argv[2])+1);
    strcpy(dirName, argv[2]);
    portNum = atoi(argv[4]);
    workerThreads = atoi(argv[6]);
    bufferSize = atoi(argv[8]);
    serverPort = atoi(argv[10]);

    serverIP = malloc(strlen(argv[12])+1);
    strcpy(serverIP, argv[12]);

    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

    //socket create and varification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1) {
        perror_exit("socket creation failed...\n");
    }

    int rc, on=1;
    //Allow socket descriptor to be reuseable
    rc = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
    if (rc < 0){
        close(sockfd);
        perror_exit("setsockopt() failed");
    }

    memset(&servaddr, '0', sizeof(servaddr));

    //struct sockaddr_in servaddr;
    //struct sockaddr serverptr = (struct sockaddr)&servaddr;
	  struct hostent *rem;

    //vriskw to serverIP
    if ((rem = gethostbyname(serverIP)) == NULL){
        perror_exit("gethostbyname1");
    }

    //dimiourgw to original socket gia ton client
    //if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) perror_exit("socket");
    //f (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0) {perror_exit("sock opt");}

    // kanw set up tis metavlites pou tha xreiastw gia ton server ston opoion tha sindethw
    servaddr.sin_family = AF_INET;
    memcpy(&servaddr.sin_addr, rem->h_addr, rem->h_length);
    servaddr.sin_port = htons(serverPort);

    //assign IP, PORT
    //servaddr.sin_family = AF_INET;
    //servaddr.sin_addr.s_addr = inet_addr(serverIP);
    //servaddr.sin_port = htons(serverPort);

    //connect the client socket to server socket
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
        perror_exit("connect");
    }
    else
        printf("connected to the server with ip %s\n", serverIP);

    //pairnw to ip address tou client
    char* clientIP;
    char tmp_buffer[256];
    int clientName = gethostname(tmp_buffer, sizeof(tmp_buffer));
    struct hostent *client_entry =  gethostbyname(tmp_buffer);
    clientIP = inet_ntoa(*((struct in_addr*)client_entry->h_addr_list[0]));

    ////stelnw LOG_ON
    char* log_on = malloc(strlen("LOG_ON <IP  , >") + 20 + numLength(portNum) + 1);

    sprintf(log_on, "LOG_ON <IP %s , %d>", clientIP, portNum);

    write(sockfd, log_on, strlen(log_on)+1);

    char buf[bufferSize];

    close(sockfd);

    ///stelnw GET_CLIENTS
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1) {
        perror_exit("socket creation failed...\n");
    }

    memset(&servaddr, '0', sizeof(servaddr));
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    memcpy(&servaddr.sin_addr, rem->h_addr, rem->h_length);
    servaddr.sin_port = htons(serverPort);
    //servaddr.sin_family = AF_INET;
    //servaddr.sin_addr.s_addr = inet_addr(serverIP);
    //servaddr.sin_port = htons(serverPort);

    //connect the client socket to server socket
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
        perror_exit("connect");
    }

    memset(buf, '0', sizeof(buf));

    write(sockfd, "GET_CLIENTS", strlen("GET_CLIENTS")+1);

    rc = recv(sockfd, buf, sizeof(buf), 0);

    //pairnw ton ari8mo twn clients pou einai sundedemenoi
    int num_of_clients;
    int cl_portNum;
    char cl_ipAddr[50];
    clientListNode* clientList = NULL;

    sscanf(buf, "CLIENT_LIST %d ", &num_of_clients);
    memset(buf, '0', sizeof(buf));
    for(int c=0; c<num_of_clients; c++){
        read(sockfd, buf, sizeof(buf));
        sscanf(buf, "< %s , %d >", cl_ipAddr, &cl_portNum);
        if((strcmp(cl_ipAddr, clientIP) == 0) && (cl_portNum == portNum)) continue;
        if(clientList == NULL)
            clientList = initializeClientsList(cl_ipAddr, cl_portNum);
        else
            clientListAppend(clientList, cl_ipAddr, cl_portNum);
        memset(buf, '0', sizeof(buf));
    }
    printClientList(clientList);
    close(sockfd);

    fd_set master;
  	fd_set read_fds;
  	int fdmax, i;

    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    connect_request(&sockfd, &cli, portNum, clientIP);
    FD_SET(sockfd, &master);

    fdmax = sockfd;
    static struct sigaction act;
    act.sa_handler = sigintHandler;
    sigfillset(&(act.sa_mask));

    while(1){
        //perimenei signal apo ton xrhsth (ctrl+C, ctrl+\)
        sigaction(SIGINT, &act, &act);
        //if(flag==0) continue;

        if(flag==1){
            ////stelnw LOG_OFF
            printf("mphka\n" );
            int servsockfd = socket(AF_INET, SOCK_STREAM, 0);
            if(servsockfd == -1) {
                perror_exit("socket\n");
            }

            memset(&servaddr, '0', sizeof(servaddr));

            // assign IP, PORT
            servaddr.sin_family = AF_INET;
            memcpy(&servaddr.sin_addr, rem->h_addr, rem->h_length);
            servaddr.sin_port = htons(serverPort);

            //connect the client socket to server socket
            if (connect(servsockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
                perror_exit("connect");
            }
            else
                printf("connected to the server with ip %s\n", serverIP);
            char* log_off = malloc(strlen("LOG_OFF <IP  , >") + strlen(clientIP) + numLength(portNum) + 1);

            sprintf(log_off, "LOG_OFF <IP %s , %d>", clientIP, portNum);
            printf("sending log_off, %s\n", log_off);
            write(servsockfd, log_off, strlen(log_off)+1);
            close(servsockfd);
            close(sockfd);

            free(serverIP);
            free(dirName);
            free(log_on);
            free(log_off);

            return 0;
        }
      	read_fds = master;

      	if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1){
      			perror_exit("select");
      	}
      	for(i = 0; i <= fdmax; i++){
      			if(FD_ISSET(i, &read_fds)){
      			    if(i == sockfd){
      					    connection_accept(&master, &fdmax, sockfd, &servaddr);
                }
      				  else{
                    if((rc = recv(i, buf, sizeof(buf), 0))<=0){
                        close(i);
                        FD_CLR(i, &master);
                    }
                    else
                        printf("%s\n", buf);
                }
      			}
      	}
    }
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <netdb.h>
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

int main(int argc,char* argv[]){

    fd_set read_fd;
    int i, j;
    int arg;
    int portNum;
    int sockfd, client_no, newsockfd, len, bytes_len, timeout, close_conn;
    int rc, on = 1, nfds = 1, current_size = 0, end_server = 0, compress_array = 0;
    char buffer[80];
    struct pollfd fds[200];
    struct sockaddr_in servaddr, client;

    while((arg = getopt(argc, argv, "p:")) != -1) {
        switch (arg) {
            case 'p':
                portNum = atoi(optarg);
                break;
            case '?':
                exit(1);
        }
    }

    //socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror_exit("socket");
    }

    //Allow socket descriptor to be reuseable
    rc = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
    if (rc < 0){
        perror("setsockopt() failed");
        close(sockfd);
        exit(-1);
    }

    //kanw to socket nonblocking
    rc = ioctl(sockfd, FIONBIO, (char *)&on);
    if (rc < 0){
        perror_exit("ioctl() failed");
        close(sockfd);
    }

    memset(&servaddr, '0', sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(portNum);

    // Binding newly created socket to given IP and verification
    if((bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        perror_exit("bind");
    }

    clientListNode* clientList = NULL;

    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) < 0) {
        printf("Listen failed...\n");
        perror_exit("listen");
    }

    //Initialize the pollfd structure
    memset(fds, 0 , sizeof(fds));

    //Set up the initial listening socket
    fds[0].fd = sockfd;
    fds[0].events = POLLIN;

    //arxikopoiw to timeout
    timeout = (3 * 60 * 1000);

    memset(&client, '0', sizeof(client));
    len = sizeof(client);

    do{
        //Call poll() and wait 3 minutes for it to complete
        //printf("Waiting on poll()...\n");
        rc = poll(fds, nfds, timeout);
        if (rc < 0){
            perror("  poll() failed");
            break;
        }
        if (rc == 0){
            printf("  poll() timed out.  End program.\n");
            break;
        }

        current_size = nfds;
        for (i = 0; i < current_size; i++){

            //vriskw ta fds pou epestrepsan POLLIN
            if(fds[i].revents == 0)
                continue;

            //an den einai POLLIN vgainw
            //log and end the server
            if(fds[i].revents != POLLIN){
                printf("  Error! revents = %d\n", fds[i].revents);
                end_server = 1;
                break;
            }
            if(fds[i].fd == sockfd)
            {
                //kanw accept ta connections
                do{
                    newsockfd = accept(sockfd, NULL, NULL);
                    if (newsockfd < 0){
                        if (errno != EWOULDBLOCK){
                            perror("  accept() failed");
                            end_server = 1;
                        }
                        break;
                    }
                    //pros8etw th nea sundesh sto fds
                    //printf("  New incoming connection - %d\n", newsockfd);
                    fds[nfds].fd = newsockfd;
                    fds[nfds].events = POLLIN;
                    nfds++;
                }while(newsockfd != -1);
            }
            else{
                close_conn = 0;

                while(1){
                    //pairnw ta data pou mou steilane
                    memset(&buffer, '0', sizeof(buffer));
                    if((rc  = recv(fds[i].fd, buffer, sizeof(buffer), 0)) < 0){
                        if(errno != EWOULDBLOCK){
                            //perror("  recv() failed");
                            close_conn = 1;
                        }
                        break;
                    }
                    char* request;
                    char* rest;
                    request = strtok(buffer, " ");

                    rest = strtok(NULL, "");

                    if(strcmp(request, "LOG_ON") == 0){

                        printf("\nlog on %s\n", rest);
                        char cl_ipAddr[100];
                        int cl_portNum;

                        sscanf(rest, "<IP %s , %d>", cl_ipAddr, &cl_portNum);

                        if(clientList == NULL)
                            clientList = initializeClientsList(cl_ipAddr, cl_portNum);
                        else
                            clientListAppend(clientList, cl_ipAddr, cl_portNum);

                        printf("send client\n");
                        send_client_to_clients(portNum, clientList, cl_ipAddr, cl_portNum);
                    }
                    else if(strcmp(request, "GET_CLIENTS") == 0){
                        printf("\nget_clients\n");
                        send_clients(clientList, fds[i].fd);
                    }
                    else if(strcmp(request, "LOG_OFF") == 0){
                        char cl_ipAddr[100];
                        int cl_portNum;

                        printf("\n");
                        printf("log_off, %s\n", rest);
                        sscanf(rest, "<IP %s , %d>", cl_ipAddr, &cl_portNum);

                        clientListNode* found_cl = findClient(clientList, cl_ipAddr, cl_portNum);
                        if(found_cl != NULL){
                            if(clientListRemove(&clientList, cl_ipAddr, cl_portNum) != NULL){
                                printf("removed %s\n", rest);
                            }
                            else{
                                clientList = NULL;
                            }
                        }
                        printClientList(clientList);
                    }
                    /////////////////////////////////////////////////////////////////////////////////////////////////////////
                    // Check to see if the connection has been closed by the client
                    if (rc == 0){
                        //printf("Connection closed with\n");
                        close_conn = 1;
                        break;
                    }
                    // Data was received
                    bytes_len = rc;
                }
                //kleinw to connection kai svhnw ton descriptor
                if (close_conn){
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    compress_array = 1;
                }
            }
        }
        // If the compress_array flag was turned on, we need
        // to squeeze together the array and decrement the number
        // of file descriptors
        if (compress_array){
            compress_array = 0;
            for (i = 0; i < nfds; i++){
                if (fds[i].fd == -1){
                    for(j = i; j < nfds; j++){
                        fds[j].fd = fds[j+1].fd;
                    }
                    i--;
                    nfds--;
                }
            }
        }
    }while(end_server == 0);

    //Clean up all of the sockets that are open
    for (i = 0; i < nfds; i++)
    {
      if(fds[i].fd >= 0)
        close(fds[i].fd);
    }

    close(sockfd);
    FreeClientList(clientList);

    return 0;
}

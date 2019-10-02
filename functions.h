//FUNCTIONS.C
void perror_exit(char *message);

int numLength(int num);

void connect_request(int *sockfd, struct sockaddr_in *my_addr, int port, char* ipAddr);

void connection_accept(fd_set *master, int *fdmax, int sockfd, struct sockaddr_in *client_addr);

void send_client_to_clients(int portNum, clientListNode* head, char* ipAddr, int cl_portNum);

void send_clients(clientListNode* head, int fd);

//LIST.C
clientListNode* initializeClientsList(char* ipAddr, int portNum);

void clientListAppend(clientListNode* head, char* ipAddr, int portNum);

clientListNode* clientListRemove(clientListNode** head, char* ipAddr, int portNum);

int numOfNodes(clientListNode* head);

clientListNode* findClient(clientListNode* head, char* ipAddr, int portNum);

void printClientList(clientListNode* head);

void FreeClientList(clientListNode* head);

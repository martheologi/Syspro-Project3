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

clientListNode* initializeClientsList(char* ipAddr, int portNum){
    clientListNode* head;

    head = malloc(sizeof(clientListNode));
    head->next = NULL;
    head->portNum = portNum;
    head->ipAddr = malloc(strlen(ipAddr)+1);
    strcpy(head->ipAddr, ipAddr);

    return head;
}

void clientListAppend(clientListNode* head, char* ipAddr, int portNum){
    clientListNode* newNode;
    clientListNode* ptr = head;

    if(ptr->portNum == portNum){
        return;
    }

    while(ptr->next!=NULL){
        if((ptr->portNum == portNum) && (strcmp(ipAddr, ptr->ipAddr) == 0)){
            return;
        }
        ptr = ptr->next;
    }

    if((ptr->portNum == portNum) && (strcmp(ipAddr, ptr->ipAddr) == 0)){
        return;
    }

    newNode = malloc(sizeof(clientListNode));
    newNode->next = NULL;
    newNode->portNum = portNum;
    newNode->ipAddr = malloc(strlen(ipAddr)+1);
    strcpy(newNode->ipAddr, ipAddr);

    ptr->next = newNode;
    return;
}

clientListNode* clientListRemove(clientListNode** head, char* ipAddr, int portNum) {
    clientListNode* next;
    clientListNode* curr = *head;
    if (curr == NULL) {
        printf("not found\n");
        return NULL;
    }
    else if((strcmp(ipAddr, (*head)->ipAddr) == 0) && (portNum == (*head)->portNum)){
        next = (*head)->next;
        free((*head)->ipAddr);
        *head = next;
        return next;
    }
    else if ((strcmp(ipAddr, curr->ipAddr) == 0) && (portNum == curr->portNum)) {
        next = curr->next;
        free(curr->ipAddr);
        free(curr);
        return next;
    }
    else{
        curr->next = clientListRemove(&curr->next, ipAddr, portNum);
        return curr;
    }
}

int numOfNodes(clientListNode* head){
    clientListNode* ptr = head;
    int c=0;

    while(ptr!=NULL){
        c++;
        ptr = ptr->next;
    }
    return c;
}

clientListNode* findClient(clientListNode* head, char* ipAddr, int portNum){
    clientListNode* ptr = head;
    while(ptr!=NULL){
        if((strcmp(ipAddr, ptr->ipAddr) == 0) && (portNum == ptr->portNum)){
            return ptr;
        }
        ptr = ptr->next;
    }
    return NULL;
}

void printClientList(clientListNode* head){
    clientListNode* ptr = head;
    int i=1;
    if(ptr == NULL)
        printf("empty list\n");

    while(ptr!=NULL){
        printf("client %d: %s %d\n", i, ptr->ipAddr, ptr->portNum);
        ptr = ptr->next;
        i++;
    }
}

void FreeClientList(clientListNode* head){
    clientListNode* current = head;
    clientListNode* temp_next;

    while(current != NULL){
        temp_next = current;
        current = current->next;
        free(temp_next->ipAddr);
        free(temp_next);
    }
}

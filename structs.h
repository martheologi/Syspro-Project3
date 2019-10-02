#ifndef __STRUCTS__
#define __STRUCTS__

typedef struct clientListNode{
    char* ipAddr;
    int portNum;
    struct clientListNode* next;   //deikths se epomeno komvo
}clientListNode;

#endif

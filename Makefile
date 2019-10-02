OBJS1 = dropbox_server.o functions.o list.o
SOURCE1 = dropbox_server.c functions.c list.c
OUT1 = dropbox_server
OBJS2 = dropbox_client.o functions.o list.o
SOURCE2 = dropbox_client.c functions.c list.c
OUT2 = dropbox_client
HEADER = structs.h functions.h
CC = gcc
FLAGS = -g3 -c

server: $(OBJS1)
	$(CC) -g $(OBJS1) -o $(OUT1)

client: $(OBJS2)
	$(CC) -g $(OBJS2) -o $(OUT2)

dropbox_client.o: dropbox_client.c
	$(CC) $(FLAGS) dropbox_client.c

dropbox_server.o: dropbox_server.c
	$(CC) $(FLAGS) dropbox_server.c

functions.o: functions.c
	$(CC) $(FLAGS) functions.c

list.o: list.c
	$(CC) $(FLAGS) list.c

clean:
	rm -f $(OBJS1) $(OUT1) $(OBJS2) $(OUT2)

CC = g++

TARGET = worker request client server

CFLAGS += -g -I/opt/local/include
#LDFLAGS += -g -lpthread
LDFLAGS += -g -lprotobuf-c -lpthread -L/opt/local/lib -L/usr/lib -lssl -lcrypto

all:	$(TARGET)

client: 	lsp.o  
	$(CC)  $(CFLAGS) lsp.o epoch_handler.o network_handler.o lspmessage.pb-c.o sample_client.c -o $@ $(LDFLAGS)
worker: worker_client.c	lsp.o  
	$(CC)  $(CFLAGS) lsp.o epoch_handler.o network_handler.o lspmessage.pb-c.o worker_client.c -o $@ $(LDFLAGS)
request: request_client.c lsp.o  
	$(CC)  $(CFLAGS) lsp.o epoch_handler.o network_handler.o lspmessage.pb-c.o request_client.c -o $@ $(LDFLAGS)



server: 	lsp.o 
	$(CC)  $(CFLAGS) lsp.o epoch_handler.o network_handler.o lspmessage.pb-c.o sample_server.c -o $@ $(LDFLAGS)

lsp.o: epoch_handler.o network_handler.o lspmessage.pb-c.o 

%.o:	%.c
	$(CC) -c $(CFLAGS) $< -o $@ 

clean:
	rm -f *.o 
	rm -f $(TARGET)


CC = g++

TARGET = worker request sample_client sample_server server test

CFLAGS += -g -I/opt/local/include
#LDFLAGS += -g -lpthread
LDFLAGS += -g -lprotobuf-c -lpthread -L/opt/local/lib -L/usr/lib -lssl -lcrypto

all:	$(TARGET)

sample_client: 	lsp.o  
	$(CC)  $(CFLAGS) lsp.o epoch_handler.o network_handler.o lspmessage.pb-c.o sample_client.c -o $@ $(LDFLAGS)
sample_server: 	lsp.o  
	$(CC)  $(CFLAGS) lsp.o epoch_handler.o network_handler.o lspmessage.pb-c.o sample_server.c -o $@ $(LDFLAGS)
worker: worker_client.c	lsp.o  
	$(CC)  $(CFLAGS) lsp.o epoch_handler.o network_handler.o lspmessage.pb-c.o worker_client.c -o $@ $(LDFLAGS)
request: request_client.c lsp.o  
	$(CC)  $(CFLAGS) lsp.o epoch_handler.o network_handler.o lspmessage.pb-c.o request_client.c -o $@ $(LDFLAGS)

server: server.cpp lsp.o
	$(CC)  $(CFLAGS) lsp.o epoch_handler.o network_handler.o lspmessage.pb-c.o server.cpp -o $@ $(LDFLAGS)

test: lsp.o
	$(CC)  $(CFLAGS) lsp.o epoch_handler.o network_handler.o lspmessage.pb-c.o template_test_1.cpp -o $@ $(LDFLAGS)

lsp.o: epoch_handler.o network_handler.o lspmessage.pb-c.o 

%.o:	%.c
	$(CC) -c $(CFLAGS) $< -o $@ 

clean:
	rm -f *.o *.txt 
	rm -f $(TARGET)


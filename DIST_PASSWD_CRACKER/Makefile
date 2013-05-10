CC = g++

TARGET = worker request server

CFLAGS += -g -I/opt/local/include
#LDFLAGS += -g -lpthread
LDFLAGS += -g -lprotobuf-c -lpthread -L/opt/local/lib -L/usr/lib -lssl -lcrypto

all:	$(TARGET)

sample_client: 	lsp.o  
	$(CC)  $(CFLAGS) lsp.o epoch_handler.o network_handler.o lspmessage.pb-c.o sample_client.c -o $@ $(LDFLAGS)
sample_server: 	lsp.o  
	$(CC)  $(CFLAGS) lsp.o epoch_handler.o network_handler.o lspmessage.pb-c.o sample_server.c -o $@ $(LDFLAGS)
worker: worker_client.cpp lsp_client.o lsp.o client_epoch_handler.o client_network_handler.o
	$(CC)  $(CFLAGS) lsp.o lsp_client.o client_epoch_handler.o client_network_handler.o lspmessage.pb-c.o worker_client.cpp -o $@ $(LDFLAGS)
request: request_client.cpp lsp.o  client_epoch_handler.o client_network_handler.o
	$(CC)  $(CFLAGS) lsp.o lsp_client.o client_epoch_handler.o client_network_handler.o lspmessage.pb-c.o request_client.cpp -o $@ $(LDFLAGS)

server: server.cpp lsp.o lsp_server.o server_epoch_handler.o server_network_handler.o
	$(CC)  $(CFLAGS) lsp.o lsp_server.o server_epoch_handler.o server_network_handler.o lspmessage.pb-c.o server.cpp -o $@ $(LDFLAGS)

test: lsp.o
	$(CC)  $(CFLAGS) lsp.o epoch_handler.o network_handler.o lspmessage.pb-c.o template_test_1.cpp -o $@ $(LDFLAGS)

lsp.o: lspmessage.pb-c.o 

%.o:	%.cpp
	$(CC) -c $(CFLAGS) $< -o $@ 

clean:
	rm -f *.o *.log 
	rm -f $(TARGET)


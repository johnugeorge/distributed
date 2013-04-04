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
worker: worker_client.cpp lsp_rpc.h lsp_client.o lsp.o client_epoch_handler.o client_network_handler.o lsp_rpc_clnt.o lsp_rpc_xdr.o
	$(CC)  $(CFLAGS) lsp.o lsp_client.o client_epoch_handler.o client_network_handler.o lspmessage.pb-c.o worker_client.cpp lsp_rpc_clnt.o lsp_rpc_xdr.o -o $@ $(LDFLAGS)
request: request_client.cpp lsp_rpc.h lsp.o  client_epoch_handler.o client_network_handler.o lsp_rpc_clnt.o lsp_rpc_xdr.o
	$(CC)  $(CFLAGS) lsp.o lsp_client.o client_epoch_handler.o client_network_handler.o lspmessage.pb-c.o request_client.cpp lsp_rpc_clnt.o lsp_rpc_xdr.o -o $@ $(LDFLAGS)

server: server.cpp lsp.o lsp_rpc.h lsp_server.o server_epoch_handler.o server_network_handler.o lsp_rpc_svc.o lsp_rpc_xdr.o
	$(CC)  $(CFLAGS) lsp.o lsp_server.o server_epoch_handler.o server_network_handler.o lspmessage.pb-c.o lsp_rpc_svc.o lsp_rpc_xdr.o server.cpp -o $@ $(LDFLAGS)

test: lsp.o
	$(CC)  $(CFLAGS) lsp.o epoch_handler.o network_handler.o lspmessage.pb-c.o template_test_1.cpp -o $@ $(LDFLAGS)

lsp_rpc_svc.o: lsp_rpc.h lsp_rpc_svc.c
	        $(CC) $(CFLAGS) -c lsp_rpc_svc.c

lsp_rpc_clnt.o: lsp_rpc.h lsp_rpc_clnt.c
	        $(CC) $(CFLAGS) -c lsp_rpc_clnt.c

lsp_rpc_xdr.o: lsp_rpc.h lsp_rpc_xdr.c
	        $(CC) $(CFLAGS) -c lsp_rpc_xdr.c

lsp.o: lspmessage.pb-c.o 

lsp_rpc.h: lsp_rpc.x
	  rpcgen -h lsp_rpc.x -o lsp_rpc.h

lsp_rpc_xdr.c: lsp_rpc.x
	  rpcgen -c lsp_rpc.x -o lsp_rpc_xdr.c

lsp_rpc_clnt.c: lsp_rpc.x
	  rpcgen -l lsp_rpc.x -o lsp_rpc_clnt.c

lsp_rpc_svc.c: lsp_rpc.x
	  rpcgen -m lsp_rpc.x -o lsp_rpc_svc.c

%.o:	%.cpp
	$(CC) -c $(CFLAGS) $< -o $@ 

clean:
	rm -f *.o *.log lsp_rpc_* lsp_rpc.h
	rm -f $(TARGET)


CC = g++
TARGET = server request worker
DEBUG = #-g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = $(DEBUG)
LIBS = -lprotobuf -lpthread -lssl -lcrypto

all: $(TARGET)
    
request: network.o lsp_client.o lsp_rpc.h lsp_rpc_clnt.o lsp_rpc_xdr.o
	$(CC) $(LFLAGS) -o $@ $@.cpp $^ $(LIBS)

worker: network.o lsp_client.o lsp_rpc.h lsp_rpc_clnt.o lsp_rpc_xdr.o
	$(CC) $(LFLAGS) -o $@ $@.cpp $^ $(LIBS)

server: network.o lsp_server.o lsp_rpc.h  lsp_rpc_svc.o lsp_rpc_xdr.o
	$(CC) $(LFLAGS) -o $@ $@.cpp $^ $(LIBS)

lsp_rpc.h: lsp_rpc.x
	          rpcgen -h lsp_rpc.x -o lsp_rpc.h

lsp_rpc_xdr.c: lsp_rpc.x
	          rpcgen -c lsp_rpc.x -o lsp_rpc_xdr.c

lsp_rpc_clnt.c: lsp_rpc.x
	          rpcgen -l lsp_rpc.x -o lsp_rpc_clnt.c

lsp_rpc_svc.c: lsp_rpc.x
	          rpcgen -m lsp_rpc.x -o lsp_rpc_svc.c

lsp_rpc_svc.o: lsp_rpc.h lsp_rpc_svc.c
	                $(CC) $(CFLAGS) -c lsp_rpc_svc.c

lsp_rpc_clnt.o: lsp_rpc.h lsp_rpc_clnt.c
	                $(CC) $(CFLAGS) -c lsp_rpc_clnt.c

lsp_rpc_xdr.o: lsp_rpc.h lsp_rpc_xdr.c
	                $(CC) $(CFLAGS) -c lsp_rpc_xdr.c

network.o : lsp_rpc.h

%.o: %.cpp
	$(CC) $(CFLAGS) $<

clean:  
	rm -f lsp_rpc_* lsp_rpc.h
	rm -f *.o 
	rm -f $(TARGET)
	rm -f *~

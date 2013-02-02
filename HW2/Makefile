CC = g++

TARGET = sample_client sample_server

CFLAGS += -g -I/opt/local/include
LDFLAGS += -g -lpthread
#LDFLAGS += -g -lprotobuf-c -L/opt/local/lib

all:	$(TARGET)

sample_client: 	lsp.o  
	$(CC)  $(CFLAGS) lsp.o epoch_handler.o sample_client.c -o $@ $(LDFLAGS)

sample_server: 	lsp.o 
	$(CC)  $(CFLAGS) lsp.o epoch_handler.o sample_server.c -o $@ $(LDFLAGS)

lsp.o: epoch_handler.o

%.o:	%.c
	$(CC) -c $(CFLAGS) $< -o $@ 

clean:
	rm -f *.o 
	rm -f $(TARGET)


CC = g++

TARGET = sample_client sample_server

CFLAGS += -g -I/opt/local/include
LDFLAGS += -g -lprotobuf-c -L/opt/local/lib

all:	$(TARGET)

sample_client: 	lsp.o lspmessage.pb-c.o 
	$(CC)  $(CFLAGS) lsp.o lspmessage.pb-c.o sample_client.c -o $@ $(LDFLAGS)

sample_server: 	lsp.o lspmessage.pb-c.o 
	$(CC)  $(CFLAGS) lsp.o lspmessage.pb-c.o sample_server.c -o $@ $(LDFLAGS)

%.o:	%.c
	$(CC) -c $(CFLAGS) $< -o $@ 

clean:
	rm -f *.o 
	rm -f $(TARGET)




#ifndef DATATYPES_H
#define DATATYPES_H

#define DEBUG false

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <queue>
#include <map>
#include <set>
#include <pthread.h>
#include <unistd.h>
#include "lsp_rpc.h"
#include <string>
#include "Queue.h"
#include <iostream>
//#include "lspmessage.pb.h"

typedef struct {
	int conn_id;
	int seq_num;
	int length;
	std::string data;
	
	public:

	inline int connid()
	{
		return conn_id;
	}
	inline int seqnum()
	{
		return seq_num;
	}

	inline std::string payload()
	{
		return data;
	}
	
	inline void set_connid(int conn)
	{
		conn_id=conn;
	}
	inline void set_seqnum(int seq)
	{
		seq_num=seq;
	}

	inline void set_payload(char* str,int len)
	{
		std::string temp(str,len );
		data=temp;
		length=len;
	}
	inline void print()
	{
		std::cout<<"----- LSP Message START-----\n";
		std::cout<<" Conn ID "<<conn_id<<"\n";
		std::cout<<" Seq No "<<seq_num<<"\n";
		//std::cout<<" Payload  "<<data<<"\n";
		std::cout<<"----- LSP Message END-----\n";
	}

}LSPMessage;



typedef enum {
    DISCONNECTED, CONNECT_SENT, CONNECTED
} Status;

typedef struct {
    const char              *host;
    unsigned int            port;
    int                     fd;
    struct sockaddr_in      *addr;
    Status                  status;
    unsigned int            id;
    unsigned int            client_id;
    unsigned int            lastSentSeq;
    unsigned int            lastReceivedSeq;
    unsigned int            lastReceivedAck;
    unsigned int            epochsSinceLastMessage;
    std::queue<LSPMessage*> outbox;
    Queue<LSPMessage*> rpcOutbox;
    unsigned long	     in_addr_int;
} Connection;

#endif

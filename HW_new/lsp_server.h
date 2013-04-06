#pragma once
#include "lsp.h"
#include "Queue.h"

typedef struct{
	LSPMessage* lspmsg;
	unsigned long addr;
	unsigned short port;
}rpcInbox_struct;

typedef struct {
    unsigned int            port;
    unsigned int            nextConnID;
    bool                    running;
    Connection              *connection;
    std::map<unsigned int,Connection*> clients;
    std::set<unsigned long>   connections;
    std::queue<LSPMessage*> inbox;
    Queue<rpcInbox_struct> rpcInbox;
    pthread_mutex_t         mutex;
    pthread_t               readThread;
    pthread_t               writeThread;
    pthread_t               epochThread; 
    pthread_t               RPCThread; 
    SVCXPRT                 *transp;
} lsp_server;

// API Methods
lsp_server* lsp_server_create(int port);
int  lsp_server_read(lsp_server* a_srv, void* pld, uint32_t* conn_id);
bool lsp_server_write(lsp_server* a_srv, void* pld, int lth, uint32_t conn_id);
bool lsp_server_close(lsp_server* a_srv, uint32_t conn_id);

// Internal Methods
void* ServerEpochThread(void *params);
void* ServerReadThread(void *params);
void* ServerWriteThread(void *params);
void* RPCThread(void *params);
extern void lsp_program_1(struct svc_req *rqstp, register SVCXPRT *transp);
LSPMessage* rpc_read_message(Connection *conn, double timeout,unsigned long& addr,unsigned short& port);
void cleanup_connection(Connection *s);

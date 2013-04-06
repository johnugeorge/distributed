#pragma once
#include "lsp.h"

typedef struct {
    Connection              *connection;
    std::queue<LSPMessage*> inbox;
    pthread_mutex_t         mutex;
    pthread_mutex_t         rpcMutex;
    pthread_t               readThread;
    pthread_t               writeThread;
    pthread_t               epochThread;
    CLIENT 		    *clnt_handle;
} lsp_client;

// API Methods
lsp_client* lsp_client_create(const char* dest, int port);
int lsp_client_read(lsp_client* a_client, uint8_t* pld);
bool lsp_client_write(lsp_client* a_client, uint8_t* pld, int lth);
bool lsp_client_close(lsp_client* a_client);

// Internal methods
void* ClientEpochThread(void *params);
void* ClientReadThread(void *params);
void* ClientWriteThread(void *params);
void cleanup_connection(Connection *s);
void cleanup_client(lsp_client *client);
void client_send_message(lsp_client* client,LSPMessage* msg);
void send_message(lsp_client* client, LSPMessage* msg);
void client_acknowledge(lsp_client* client);
LSPMessage* rpc_read_message(lsp_client* client, double timeout);

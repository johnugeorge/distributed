#include<iostream>
#include<sys/socket.h>
#include<sys/time.h>
#include<error.h>
#include"lsp_server.h"
#include"lsp_client.h"


void c_epoch_timer(void*);
void s_epoch_timer(void*);
void c_handle_epoch(lsp_client* client);
void s_handle_epoch(lsp_server* server);

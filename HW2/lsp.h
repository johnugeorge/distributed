#pragma once


#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <errno.h>
#include <strings.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

// Global Parameters. For both server and clients.

#define _EPOCH_LTH 2.0
#define _EPOCH_CNT 5;
#define _DROP_RATE 0.0;
#define  MAX_CLIENTS 20

void lsp_set_epoch_lth(double lth);
void lsp_set_epoch_cnt(int cnt);
void lsp_set_drop_rate(double rate);

typedef struct
{
int connection_id;
int seq_number;
char* data;
}pckt_fmt;

typedef enum pckt_type
{
	DATA_PCKT=1,
	DATA_ACK =2,
	CONN_REQ =3,
	CONN_ACK =4
}pckt_type;


typedef enum conn_type
{
	CONN_REQ_SENT=1,
	CONN_REQ_ACK_RCVD=2,
	CONN_REQ_RCVD=3,
	CONN_REQ_ACK_SENT=4,
	DATA_SENT=5,
	DATA_ACK_RCD=6,
	DATA_RCVD=7,
	DATA_ACK_SENT=8
}conn_type;

typedef struct
{
int connection_id;
int socket_fd;
pckt_type last_pkt_rcvd;
pckt_type last_pkt_sent;
conn_type conn_state;
pckt_fmt* last_pckt_rcvd;
pckt_fmt* last_pckt_sent;
} client_info;



typedef struct
{
int connection_id;
int socket_fd;
pckt_type last_pkt_rcvd;
pckt_type last_pkt_sent;
conn_type conn_state;
pckt_fmt* last_pckt_rcvd;
pckt_fmt* last_pckt_sent;
	
} lsp_client;

lsp_client* lsp_client_create(const char* dest, int port);
int lsp_client_read(lsp_client* a_client, char* pld);
bool lsp_client_write(lsp_client* a_client, char* pld, int lth);
bool lsp_client_close(lsp_client* a_client);

typedef struct
{
client_info* client_conn[MAX_CLIENTS];
} lsp_server;


lsp_server* lsp_server_create(int port);
int  lsp_server_read(lsp_server* a_srv, void* pld, int* conn_id);
bool lsp_server_write(lsp_server* a_srv, void* pld, int lth, int conn_id);
bool lsp_server_close(lsp_server* a_srv, int conn_id);

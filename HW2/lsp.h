#pragma once


#include <stdio.h>
#include <iostream>
#include <stdint.h>
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
#include <queue>
#include <map>
#include <arpa/inet.h>
// Global Parameters. For both server and clients.

#define _EPOCH_LTH 2.0
#define _EPOCH_CNT 5
#define _DROP_RATE 0.0
#define  MAX_CLIENTS 20
#define MAX_PAYLOAD_SIZE 1000
#define cout cout<<pthread_self()<<" : " 

#define PRINT_PACKET(pkt,dir) \
	cout<< "==========START=======================\n"; \
	cout<< " ----"<<dir<<" Packet Info----\n"; \
	cout<< " Conn_Id:  "<<pkt.conn_id<<"\n"; \
        cout<< " Seq_no:  "<<pkt.seq_no<<"\n"; \
        cout<< " Payload: "<<pkt.data<<"\n"; \
	cout<< "==========END=======================\n";

void lsp_set_epoch_lth(double lth);
void lsp_set_epoch_cnt(int cnt);
void lsp_set_drop_rate(double rate);
double lsp_get_epoch_lth();
int lsp_get_epoch_cnt();
double lsp_get_epoch_rate();

typedef struct
{
int conn_id;
int seq_no;
char* data;
}pckt_fmt;

typedef struct conn_arg
{
	int conn_id;
	int seq_no;
}conn_arg;


typedef struct
{
	bool operator() (const conn_arg& a,const conn_arg& b)
	{
		if(a.conn_id == b.conn_id)
			return (a.seq_no < b.seq_no);
		else
			return (a.conn_id < b.conn_id);
	}
}conn_comp;

typedef std::map<conn_arg,bool,conn_comp> conn_seqno_map;
typedef enum pckt_type
{
	DATA_PCKT=1,
	DATA_ACK =2,
	CONN_REQ =3,
	CONN_ACK =4,
	DATA_PCKT_RESEND=5
}pckt_type;

inline std::ostream& operator<< (std::ostream& os, pckt_type var) {

	switch (var) {
		case DATA_PCKT:
			return os << "DATA PACKET";
		case DATA_ACK:
			return os << "DATA ACK";
		case CONN_REQ:
			return os << "CONNECTION REQUEST";
		case CONN_ACK:
			return os << "CONNECTION ACK";
	}
	return os;
}


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

typedef struct client_info
{
int conn_id;
int socket_fd;
int seq_no;
struct sockaddr_storage addr;
pckt_type last_pkt_rcvd;
pckt_type last_pkt_sent;
conn_type conn_state;
pckt_fmt last_pckt_rcvd;
pckt_fmt* last_pckt_sent;
std::queue<pckt_fmt> inbox_queue;
std::queue<pckt_fmt> outbox_queue;
conn_seqno_map conn_map;
bool first_data_rcvd;
bool first_data_sent;
int last_seq_no_rcvd;
client_info()
{
seq_no=0;
conn_id=0;
last_seq_no_rcvd=0;
first_data_rcvd=false;
first_data_sent=false;
}
} client_info;


typedef struct lsp_client
{
int conn_id;
int socket_fd;
int seq_no;
struct addrinfo* serv_info;
pckt_type last_pkt_rcvd;
pckt_type last_pkt_sent;
conn_type conn_state;
pckt_fmt last_pckt_rcvd;
pckt_fmt last_pckt_sent;
std::queue<pckt_fmt> inbox_queue;
std::queue<pckt_fmt> outbox_queue;
conn_seqno_map conn_map;
bool first_data_rcvd;
bool first_data_sent;
int last_seq_no_rcvd;
lsp_client()
{
conn_state=CONN_REQ_SENT;
seq_no=0;
conn_id=0;
last_seq_no_rcvd=0;
first_data_rcvd=false;
first_data_sent=false;
last_pckt_sent.data=NULL;
}
} lsp_client;

typedef std::map<int ,client_info*> client_info_map;

lsp_client* lsp_client_create(const char* dest, int port);
int lsp_client_read(lsp_client* a_client, uint8_t* pld);
bool lsp_client_write(lsp_client* a_client, uint8_t* pld, int lth);
bool lsp_client_close(lsp_client* a_client);

typedef struct lsp_server
{
client_info_map client_conn_info;
int next_free_conn_id;
int socket_fd;
lsp_server()
{
	next_free_conn_id=1;
}
} lsp_server;


lsp_server* lsp_server_create(int port);
int  lsp_server_read(lsp_server* a_srv, void* pld, uint32_t* conn_id);
bool lsp_server_write(lsp_server* a_srv, void* pld, int lth, uint32_t conn_id);
bool lsp_server_close(lsp_server* a_srv, uint32_t conn_id);

uint8_t* message_encode(int conn_id,int seq_no,const char* payload,int& outlength);
int message_decode(int len,uint8_t* buf,pckt_fmt& pkt);
void client_send(lsp_client* client,pckt_type pkt_type,int seq_no=0,const char *payload="",int length=0);
void server_send(lsp_server* server,pckt_type pkt_type,int client_conn_id,int seq_no=0,const char *payload="",int length=0);


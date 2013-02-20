#include"lsp.h"


typedef struct lsp_client
{
pthread_mutex_t lock;
int conn_id;
int socket_fd;
int seq_no;
struct addrinfo* serv_info;
pckt_type last_pkt_rcvd;
pckt_type last_pkt_sent;
conn_type conn_state;
pckt_fmt last_pckt_rcvd;
pckt_fmt last_pckt_sent;
Queue<inbox_struct> inbox_queue;
Queue<outbox_struct> outbox_queue;
conn_seqno_map conn_map;
bool first_data_rcvd;
bool first_data_sent;
int last_seq_no_rcvd;
int no_epochs_elapsed;
lsp_client()
{
last_pckt_sent.data=NULL;
no_epochs_elapsed=0;
conn_state=CONN_WAIT;
seq_no=0;
conn_id=0;
last_seq_no_rcvd=0;
first_data_rcvd=false;
first_data_sent=false;
last_pckt_sent.data=NULL;
}
} lsp_client;


lsp_client* lsp_client_create(const char* dest, int port);
int lsp_client_read(lsp_client* a_client, uint8_t* pld);
bool lsp_client_write(lsp_client* a_client, uint8_t* pld, int lth);
bool lsp_client_close(lsp_client* a_client);




void client_send(lsp_client* client,pckt_type pkt_type,int seq_no=0,const char *payload="",int length=0);
void set_ack_status(lsp_client* client,conn_arg arg, bool value);
bool get_ack_status(lsp_client* client,conn_arg arg);


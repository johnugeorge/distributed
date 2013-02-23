#include"lsp.h"


typedef struct client_info
{
int conn_id;
int socket_fd;
int seq_no;
struct sockaddr_storage addr;
//pckt_type last_pkt_rcvd;
//pckt_type last_pkt_sent;
conn_type conn_state;
//pckt_fmt last_pckt_rcvd;
pckt_fmt last_pckt_sent;
//Queue<inbox_struct> inbox_queue;
Queue<outbox_struct> outbox_queue;
conn_seqno_map conn_map;
bool first_data_rcvd;
bool first_data_sent;
int last_seq_no_rcvd;
int no_epochs_elapsed;
client_info()
{
last_pckt_sent.data=NULL;
seq_no=0;
conn_state=CONN_WAIT;
conn_id=0;
last_seq_no_rcvd=0;
first_data_rcvd=false;
first_data_sent=false;
no_epochs_elapsed=0;
}
} client_info;


typedef std::map<int ,client_info*> client_info_map;

typedef struct lsp_server
{
client_info_map client_conn_info;
Queue<inbox_struct> inbox_queue;
pthread_mutex_t lock;
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


void server_send(lsp_server* server,pckt_type pkt_type,int client_conn_id,int seq_no=0,const char *payload="",int length=0);
void set_ack_status(lsp_server* server,client_info* client,conn_arg arg, bool value);
bool get_ack_status(lsp_server* server,client_info* client,conn_arg arg);


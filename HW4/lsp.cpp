#include<pthread.h>
#include "lsp.h"
#include "epoch_handler.h"
#include "network_handler.h"
#include "lspmessage.pb-c.h"

using namespace std;

// values are read from lsp.conf during startup
// of server,worker and requester.
// Change values in lsp.conf and restart
double epoch_lth; 
int epoch_cnt; 
double drop_rate;
int loglevel; 
bool to_file; 
thread_info thread_info_map;

pthread_mutex_t global_mutex;
pthread_cond_t global_created;

Configuration config;
/*
 *
 *
 * LSP RELATED FUNCTIONS
 *
 *
 */  

void lsp_set_epoch_lth(double lth)
{
  epoch_lth = lth;
}

void lsp_set_epoch_cnt(int cnt)
{
  epoch_cnt = cnt;
}

void lsp_set_drop_rate(double rate)
{
  drop_rate = rate;
}

void lsp_set_log_level(int l)
{
  loglevel = l;
}

void set_to_file(bool t)
{
  to_file = t;
}


double lsp_get_epoch_lth()
{
  return epoch_lth;
}

int lsp_get_epoch_cnt()
{
  return epoch_cnt;
}

double lsp_get_drop_rate()
{
  return drop_rate;
}

bool get_to_file()
{
  return to_file;
}



uint8_t* message_encode(int conn_id,int seq_no,const char* payload,int& outlength)
{

int len;
LSPMessage msg = LSPMESSAGE__INIT;
msg.connid = conn_id;
msg.seqnum = seq_no;
msg.payload.data = (uint8_t *)malloc(sizeof(uint8_t) * (strlen(payload)+1));
msg.payload.len = strlen(payload)+1;
memcpy(msg.payload.data, payload, (strlen(payload)+1)*sizeof(uint8_t));
len = lspmessage__get_packed_size(&msg);
//PRINT(LOG_DEBUG,"len" <<len<<" strlen "<<strlen(payload);
uint8_t* buf = (uint8_t *)malloc(len);
lspmessage__pack(&msg, buf);
outlength=len;
free(msg.payload.data);
return buf;

}

int message_decode(int len,uint8_t* buf, pckt_fmt& pkt)
{

LSPMessage* msg=lspmessage__unpack(NULL, len, buf);
pkt.conn_id=msg->connid;
pkt.seq_no=msg->seqnum;
memcpy(pkt.data,msg->payload.data,msg->payload.len);
//PRINT(LOG_DEBUG," Conn_id "<<pkt.conn_id<<"\n";
//PRINT(LOG_DEBUG," Seq Num "<<pkt.seq_no<<"\n";
//PRINT(LOG_DEBUG," payload data"<<pkt.data<<"\n";
//PRINT(LOG_DEBUG," payload len"<<msg->payload.len<<"\n";
//if(pkt.data[0]=='\0')PRINT(LOG_DEBUG,"true" ;else PRINT(LOG_DEBUG,"false";
return msg->payload.len;
}




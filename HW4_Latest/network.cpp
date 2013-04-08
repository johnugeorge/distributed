

#include "network.h"

double drop_rate = _DROP_RATE;

#define PACKET_SIZE 2048

Connection* network_setup_server(int port){
    Connection *conn = new Connection();
    
    return conn;
}

Connection* network_make_connection(const char *host, int port){
    
    Connection *c = new Connection();
    
    return c;   
}

// send a connection request
bool network_send_connection_request(Connection *conn){
    LSPMessage *msg = network_build_message(0,0,NULL,0);    
    if(network_send_message(conn,msg)) {
        conn->status = CONNECT_SENT;
        return true;
    } else {
        return false;
    }
}


// acknowledge the last received message
bool network_acknowledge(Connection *conn){
    LSPMessage *msg = network_build_message(conn->id, conn->lastReceivedSeq, NULL, 0);
    
    return network_send_message(conn, msg);
}

bool network_send_message(Connection *conn, LSPMessage *msg){
  //instead of sending the msg using sendto, we will just push it into
  //this connection's global outbox queue
  conn->rpcOutbox.putq(msg);
  if(DEBUG) printf(" making RPC call %d\n",conn->client_id);

  if(DEBUG)printf(" making RPC call %d %d\n",conn->client_id, msg->seq_num);
  int conn_id=msg->conn_id;
  int seq_no=msg->seq_num;
  std::string payload=msg->data;
  LSPMessage1 pkt;
  pkt.connid=conn_id;
  pkt.seqnum=seq_no;
  pkt.payload=(char*)payload.c_str();
  int result;
  int addr= conn->in_addr_int;
  struct sockaddr_in new_addr;
  new_addr.sin_addr.s_addr=addr;
  char host[128];
  sprintf(host,"%s",inet_ntoa(new_addr.sin_addr));
  if(DEBUG) printf(" Address %s \n",inet_ntoa(new_addr.sin_addr));

  int ans = callrpc(host, conn->client_id, 1,
		  (__const u_long) 1, (__const xdrproc_t) xdr_LSPMessage1, (char*)&pkt, (__const xdrproc_t) xdr_int, (char*)&result);

  if(DEBUG)printf(" RPC called\n");
/*  if ((enum clnt_stat) ans != RPC_SUCCESS) {
	  fprintf(stderr, "call callrpc: %d %d",conn->client_id,msg->conn_id);
	  clnt_perrno((enum clnt_stat)ans);
	  fprintf(stderr, "\n");
  }
*/
}



LSPMessage* network_build_message(int id, int seq, uint8_t *pld, int len){
    // create the LSPMessage data structure and fill it in
    
    LSPMessage *msg = new LSPMessage();
    
    msg->set_connid(id);
    msg->set_seqnum(seq);
    msg->set_payload((char*)pld,len);
    return msg;
    
}

// configure the network drop rate
void network_set_drop_rate(double percent){
    if(percent >= 0 && percent <= 1)
        drop_rate = percent;
}

// use the drop rate to determine if we should send/receive this packet
bool network_should_drop(){
    return (rand() / (double)RAND_MAX) < drop_rate;
}

struct timeval network_get_timeval(double seconds){
    // create the timeval structure needed for the timeout in the select call for reading from a socket
    timeval t;
    t.tv_sec = (long)seconds;
    t.tv_usec = (seconds - (long)seconds) * 1000000; // remainder in s to us
    return t;
}


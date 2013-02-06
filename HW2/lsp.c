#include<pthread.h>
#include "lsp.h"
#include "epoch_handler.h"
#include "network_handler.h"
#include "lspmessage.pb-c.h"

using namespace std;
double epoch_lth = _EPOCH_LTH;
int epoch_cnt = _EPOCH_CNT;
double drop_rate = _DROP_RATE;

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



/*
 *
 *
 * CLIENT RELATED FUNCTIONS
 *
 *
 */  

lsp_client* lsp_client_create(const char* src, int port)
{
  struct addrinfo hints, *servinfo;
  int sockfd, yes=1;

	// first, load up address structs with getaddrinfo():

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

	char buf[5];
	sprintf(buf,"%d",port);
	const char* udpPort = (char*) &buf;
	getaddrinfo(src, udpPort, &hints, &servinfo);

	if ((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1){
		perror("server: socket");
		sockfd = 0;
	}

  /*
   * code to send request to server goes here 
   */

  //start the client side epoch
  pthread_t client_epoch;
  pthread_t client_network_handler;


 void *c_epoch_timer(void*);
 void *c_network_handler(void*);


  lsp_client* new_client = new lsp_client;

  new_client->socket_fd=sockfd;
  new_client->serv_info=servinfo;
  client_send(new_client,CONN_REQ);
  pthread_create(&client_network_handler, NULL, c_network_handler, (void*) new_client);
  pthread_create(&client_epoch, NULL, c_epoch_timer, (void*) new_client);



  while(new_client->conn_state == CONN_REQ_SENT){;}


//  pthread_join(client_epoch, NULL);
  return new_client;
}

int lsp_client_read(lsp_client* a_client, uint8_t* pld)
{
 if(a_client == NULL)
 {
	 cout<<" Client Ptr is NULL.exiting ";
         exit(1);
 }
 else
 {
	 if(!(a_client->inbox_queue.empty()))
	 {

		 char* ptr=(a_client->inbox_queue.front()).data;
		 cout<<" Size of msg "<<strlen(ptr);
		 a_client->inbox_queue.pop();
		 memcpy(pld,ptr,strlen(ptr));
		 free(ptr);
         }

 }
}

bool lsp_client_write(lsp_client* a_client, uint8_t* pld, int lth)
{
	int len;
	//uint8_t* buff =message_encode(a_client->conn_,,a_client->seq_no,pld,len);
        conn_arg conn_argv;
        conn_argv.conn_id=a_client->conn_id;
	conn_argv.seq_no=a_client->seq_no;
	pckt_fmt pkt;
	pkt.conn_id=a_client->conn_id;
	pkt.seq_no=a_client->seq_no;
	pkt.data=(char*)malloc(lth);
	memcpy(pkt.data,pld,lth);
	//PRINT_PACKET(pkt,"SEND")
	cout<<" Lsp_client_write "<<a_client->conn_id<<" conn state "<< (a_client->conn_state) << " data sent flag "<<a_client->first_data_sent <<" ack status "<<(a_client->conn_map)[conn_argv]<<" prev seq_no :"<<a_client->seq_no<<"\n";
	if((a_client->conn_state != CONN_REQ_SENT) && (((a_client->conn_map)[conn_argv]==true || a_client->first_data_sent ==false )))
	{
		client_send(a_client,DATA_PCKT,a_client->seq_no,(char*)pld,lth);
		if(a_client->last_pckt_sent.data != NULL)
		{
			free(a_client->last_pckt_sent.data);
		}
		a_client->last_pckt_sent.data=(char*)malloc(lth);
		a_client->last_pckt_sent.conn_id=a_client->conn_id;
		a_client->last_pckt_sent.seq_no=a_client->seq_no;
		memcpy(a_client->last_pckt_sent.data,pld,lth);
		free(pkt.data);

        }
	else
	{
		cout<<" Ack not recieved for earlier seq no "<< a_client->seq_no <<" .Now, outbox size is "<<a_client->outbox_queue.size()<<"\n";
		a_client->outbox_queue.push(pkt);

	}

}

bool lsp_client_close(lsp_client* a_client)
{
	close(a_client->socket_fd);
}

/*
 *
 *
 * SERVER RELATED FUNCTIONS
 *
 *
 */  


lsp_server* lsp_server_create(int port)
{
	struct addrinfo hints, *servinfo;
	int sockfd,yes=1;

	// first, load up address structs with getaddrinfo():

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

	char buf[5];
	sprintf(buf,"%d",port);
	const char* udpPort = (char*) &buf;
	getaddrinfo(NULL, udpPort, &hints, &servinfo);
	
	if ((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1){
		perror("server: socket");
		sockfd = 0;
	}
	
	cout<<" socket Fd "<<sockfd;
	if (sockfd && setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1){
		perror("setsockopt");
		sockfd =0;
	}

	if (sockfd && bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1){
		close(sockfd);
		perror("server:bind");
		sockfd = 0;
	}

	freeaddrinfo(servinfo);
	/*int len;
	uint8_t* buff =message_encode(1,2,"j 3434f343rf34",len);
	cout<<" outlen "<<len;
	pckt_fmt pkt;
	pkt.data=(char*)malloc(106);
	message_decode(21,buff,pkt);
	//if(pkt.data[0]=='\0')cout<<"true"<< strlen(pkt.data);else cout<<"false"<<strlen(pkt.data);*/
	pthread_t server_network_handler;
	void *s_network_handler(void*);


	lsp_server* new_server = new lsp_server;
	new_server->socket_fd=sockfd;
	pthread_create(&server_network_handler, NULL, s_network_handler, (void*) new_server);
}

int lsp_server_read(lsp_server* a_srv, void* pld, uint32_t* conn_id)
{

}

bool lsp_server_write(lsp_server* a_srv, void* pld, int lth, uint32_t conn_id)
{

}

bool lsp_server_close(lsp_server* a_srv, uint32_t conn_id)
{

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
//cout<<"len" <<len<<" strlen "<<strlen(payload);
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
//cout<<" Conn_id "<<pkt.conn_id<<"\n";
//cout<<" Seq Num "<<pkt.seq_no<<"\n";
//cout<<" payload data"<<pkt.data<<"\n";
//cout<<" payload len"<<msg->payload.len<<"\n";
//if(pkt.data[0]=='\0')cout<<"true" ;else cout<<"false";
return msg->payload.len;
}

void client_send(lsp_client* client,pckt_type pkt_type,int seq_no,const char *payload,int length)
{
	cout<<" In client send "<<client->conn_id<<"\n";
	int numbytes;
	uint8_t* buff;
	int len=0;
	string s="NIL";
	pckt_fmt pkt;
	pkt.conn_id=client->conn_id;
	conn_arg conn_argv;
	conn_argv.conn_id=client->conn_id;
	if(pkt_type== DATA_PCKT)
	{
                client->first_data_sent =true;
		client->seq_no++;
		buff=message_encode(client->conn_id,client->seq_no,payload,len);
	        conn_argv.seq_no=client->seq_no;
		pkt.seq_no=client->seq_no;
		pkt.data=(char*)payload;
	}
	else if(pkt_type == CONN_REQ)
	{
		buff=message_encode(client->conn_id,0,"\0",len);
		conn_argv.seq_no=0;
		pkt.seq_no=0;
		pkt.data=(char*)s.c_str();


	}
	else if(pkt_type == DATA_ACK)
	{
	        buff = message_encode(client->conn_id,seq_no,"\0",len);
		conn_argv.seq_no=seq_no;
		pkt.seq_no=seq_no;
		pkt.data=(char*)s.c_str();

	}
	else if(pkt_type == DATA_PCKT_RESEND)
	{
		buff=message_encode(client->conn_id,seq_no,payload,len);
		conn_argv.seq_no=seq_no;
		pkt.seq_no=seq_no;
		pkt.data=(char*)payload;
	}
	else
	{
		printf("Error in pkt type");
		exit(1);
	}
       (client->conn_map)[conn_argv]=false;
	PRINT_PACKET(pkt,"SEND")
	if ((numbytes = sendto(client->socket_fd, buff, len, 0,
					client->serv_info->ai_addr, client->serv_info->ai_addrlen)) == -1) {
		perror("client: sendto");
		exit(1);
	}

	cout<<"sent "<<numbytes<<" bytes of pkt type "<<pkt_type<<"\n";
	free(buff);
}


void server_send(lsp_server* server,pckt_type pkt_type,int client_conn_id,int seq_no,const char *payload,int length)
{
	int numbytes;
	uint8_t* buff;
	int len =0 ;
	client_info* client_conn=server->client_conn_info[client_conn_id];
	cout<<" In server send "<<client_conn->conn_id;
	conn_arg conn_argv;
	conn_argv.conn_id=client_conn_id;
	if(pkt_type== DATA_PCKT)
	{

		buff=message_encode(client_conn->conn_id,client_conn->seq_no,payload,len);
	        conn_argv.seq_no=client_conn->seq_no;
		client_conn->seq_no++;
	}
	else if(pkt_type == CONN_ACK)
	{
		buff=message_encode(client_conn->conn_id,0,"\0",len);
		conn_argv.seq_no=0;

	}
	else if(pkt_type == DATA_ACK)
	{
	        buff = message_encode(client_conn->conn_id,seq_no,"\0",len);
		conn_argv.seq_no=seq_no;
	}
	else
	{
		printf("Error in pkt type");
		exit(1);
	}
       (client_conn->conn_map)[conn_argv]=false;
	
	if ((numbytes = sendto(server->socket_fd, buff, len, 0,
					(struct sockaddr *)&client_conn->addr, sizeof(client_conn->addr))) == -1) {
		perror("client: sendto");
		exit(1);
	}

	cout<<"sent "<< numbytes<<" bytes of pkt_type "<<pkt_type<<"\n";
	free(buff);
}

#include "lsp_client.h"
pthread_t client_epoch;

pthread_t client_network_handler;


/*
 *
 *
 * CLIENT RELATED FUNCTIONS
 *
 *
 */  



/* lsp_client_create
 * Client waits for configured epoch lengths to receive 
 * connection response from server. Return false if client doesn’t get any 
 * reply otherwise return a pointer to a struct of type lsp client.
 */

lsp_client* lsp_client_create(const char* src, int port)
{
  struct addrinfo hints, *servinfo;
  thread_info_map[pthread_self()]=" MAIN CLIENT THREAD            :";
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


  //start the client side epoch
 void *c_epoch_timer(void*);
 void *c_network_handler(void*);


  lsp_client* new_client = new lsp_client;

  new_client->socket_fd=sockfd;
  new_client->serv_info=servinfo;
  new_client->conn_state = CONN_REQ_SENT;
  client_send(new_client,CONN_REQ);
  pthread_create(&client_network_handler, NULL, c_network_handler, (void*) new_client);
  pthread_create(&client_epoch, NULL, c_epoch_timer, (void*) new_client);


  struct timespec  to;
  struct timeval    tp;
  pthread_mutex_lock(&global_mutex);
  int rc =  gettimeofday(&tp, NULL);
  to.tv_sec = tp.tv_sec + lsp_get_epoch_lth()*lsp_get_epoch_cnt();
  to.tv_nsec = tp.tv_usec * 1000;
  while (new_client->conn_state == CONN_REQ_SENT) {
	  int err = pthread_cond_timedwait(&global_created, &global_mutex, &to);
	  if (err == ETIMEDOUT) {
		  PRINT(LOG_CRIT," Server not responded to Conn Request");
		  //close(sockfd);
		  return NULL;
		  // break;
	  }
  }
  pthread_mutex_unlock(&global_mutex);


  return new_client;
}
/*lsp_client_read 
 * Pop message from the queue 
 * if payload_size ==0 -> disconnection message
 *                        send -1 to application
 * else
 *     populate connection id and payload 
 *     send payload_size as return parameter to application*/

int lsp_client_read(lsp_client* a_client, uint8_t* pld)
{
 if(a_client == NULL)
 {
	 PRINT(LOG_INFO," Client Ptr is NULL.exiting ");
         exit(1);
 }
 else
 {
	 if(!(a_client->inbox_queue.empty()))
	 {
		 PRINT(LOG_DEBUG," Inbox_size "<<a_client->inbox_queue.size());
		 inbox_struct inbox;
		 a_client->inbox_queue.getq(inbox);
		 int len=inbox.payload_size;
		 if(len!=0)
		 {
		 char* ptr=inbox.pkt.data;
		 //a_client->inbox_queue.pop();
		 memcpy(pld,ptr,strlen(ptr)+1);
		 free(ptr);
		 return len;
		 }
		 else
		 {
			 pld=NULL;
			 return -1;
		 }
         }
	 return 0;
 }
}

/*lsp_client_write:  If last message which was sent from the client, is not acknowledged,
 *                     Put into outbox queue of the client
 *                          Else
 *                     Send the packet to the server.
 *
 */

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
	pkt.data=(char*)malloc(lth + 1);
	memcpy(pkt.data,pld,lth + 1);
	//PRINT_PACKET(pkt,"SEND")
	PRINT(LOG_DEBUG," Lsp_client_write "<<a_client->conn_id<<" conn state "<< (a_client->conn_state) << " data sent flag "<<a_client->first_data_sent <<" ack status "<<get_ack_status(a_client,conn_argv)<<" prev seq_no :"<<a_client->seq_no<<" length "<<lth<<"\n");
	if((a_client->conn_state == CONN_REQ_ACK_RCVD) && ((get_ack_status(a_client,conn_argv)==true || a_client->first_data_sent ==false )))
	{
		client_send(a_client,DATA_PCKT,a_client->seq_no,(char*)pld,lth);
		if(a_client->last_pckt_sent.data != NULL)
		{
			free(a_client->last_pckt_sent.data);
		}
		a_client->last_pckt_sent.data=(char*)malloc(lth+1);
		a_client->last_pckt_sent.conn_id=a_client->conn_id;
		a_client->last_pckt_sent.seq_no=a_client->seq_no;
		memcpy(a_client->last_pckt_sent.data,pld,lth +1);
		free(pkt.data);

        }
	else
	{
		PRINT(LOG_DEBUG," Ack not recieved for earlier seq no "<< a_client->seq_no <<" .Now, outbox size is "<<a_client->outbox_queue.size()<<"\n");
		//		a_client->seq_no++;
		//pkt.seq_no=a_client->seq_no;
		outbox_struct outbox;
		outbox.pkt=pkt;
		outbox.payload_size=lth;
		a_client->outbox_queue.putq(outbox);

	}

}
/* close client connection*/

bool lsp_client_close(lsp_client* a_client)
{
       // pthread_join(client_epoch, NULL);
        a_client->closed=true;
	close(a_client->socket_fd);
}

/* Depending on the packet type,calculate seq no
 * Marshall the packet to create buffer to be sent
 * and call sendto*/
void client_send(lsp_client* client,pckt_type pkt_type,int seq_no,const char *payload,int length)
{
	PRINT(LOG_DEBUG," In client send "<<client->conn_id<<"\n");
	int numbytes;
	uint8_t* buff;
	int len=0;
	char* s="NIL";
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
       		//(client->conn_map)[conn_argv]=false;
	        set_ack_status(client,conn_argv,false);
	}
	else if(pkt_type == CONN_REQ)
	{
		buff=message_encode(client->conn_id,0,"\0",len);
		conn_argv.seq_no=0;
		pkt.seq_no=0;
		pkt.data=s;
       		//(client->conn_map)[conn_argv]=false;
	        set_ack_status(client,conn_argv,false);


	}
	else if(pkt_type == DATA_ACK)
	{
	        buff = message_encode(client->conn_id,seq_no,"\0",len);
		conn_argv.seq_no=seq_no;
		pkt.seq_no=seq_no;
		pkt.data=s;

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
		PRINT(LOG_INFO,"Error in pkt type");
		exit(1);
	}
	PRINT_PACKET(pkt,"SEND")
		if(client->closed != true)
		{
			if ((numbytes = sendto(client->socket_fd, buff, len, 0,
							client->serv_info->ai_addr, client->serv_info->ai_addrlen)) == -1) 
			{
				perror("client: sendto");
				exit(1);
			}
		}
	PRINT(LOG_DEBUG,"sent "<<numbytes<<" bytes of pkt type "<<pkt_type<<"\n");
	free(buff);
}

/*Get Ack Status for Seq No which was received*/
bool get_ack_status(lsp_client* client,conn_arg arg)
{
	bool value = (client->conn_map)[arg];
	return value;
}
/*Set Ack Status for Seq No which was received*/
void set_ack_status(lsp_client* client,conn_arg arg, bool value)
{

	(client->conn_map)[arg]=value;
}



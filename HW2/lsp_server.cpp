#include "lsp_server.h"

pthread_t server_epoch;
pthread_t server_network_handler;


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
	thread_info_map[pthread_self()]=" MAIN SERVER THREAD           :";
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
	PRINT(LOG_DEBUG," outlen "<<len;
	pckt_fmt pkt;
	pkt.data=(char*)malloc(106);
	message_decode(21,buff,pkt);
	//if(pkt.data[0]=='\0')PRINT(LOG_DEBUG,"true"<< strlen(pkt.data);else PRINT(LOG_DEBUG,"false"<<strlen(pkt.data);*/
	void *s_network_handler(void*);
	void *s_epoch_timer(void*);


	lsp_server* new_server = new lsp_server;
	new_server->socket_fd=sockfd;
	pthread_create(&server_network_handler, NULL, s_network_handler, (void*) new_server);
  	pthread_create(&server_epoch, NULL, s_epoch_timer, (void*) new_server);
	return new_server;
}

int lsp_server_read(lsp_server* a_srv, void* pld, uint32_t* conn_id)
{
 if(a_srv == NULL)
 {
	 PRINT(LOG_INFO," Server Ptr is NULL.exiting ");
         exit(1);
 }
 else
 {
	 if(!(a_srv->inbox_queue.empty()))
	 {
		 PRINT(LOG_DEBUG," Inbox_size "<<a_srv->inbox_queue.size());
                 inbox_struct inbox;
	         a_srv->inbox_queue.getq(inbox);	 
		 PRINT(LOG_DEBUG," Inbox_size after dequeing "<<a_srv->inbox_queue.size());
		 int len =inbox.payload_size;
		 if(len != 0)
		 {
			 *conn_id= inbox.pkt.conn_id;
			 char* ptr=inbox.pkt.data;
			 //a_srv->inbox_queue.pop();
			 memcpy(pld,ptr,strlen(ptr) + 1);
			 free(ptr);
			 PRINT(LOG_DEBUG," Returning "<<(char*)pld<<" len "<<len<<" conn id "<<*conn_id);
			 return len;
		 }
		 else
		 {
			 pld=NULL;
			 *conn_id=inbox.pkt.conn_id;
			 return -1;
		 }
         }

 }

}

bool lsp_server_write(lsp_server* a_srv, void* pld, int lth, uint32_t conn_id)
{
	int len;
	if(a_srv == NULL)
	{
		PRINT(LOG_INFO," Server Ptr is NULL.exiting ");
		exit(1);
	}
	else
	{
		conn_arg conn_argv;
		client_info* a_client=a_srv->client_conn_info[conn_id];
		if( a_client ==NULL)
		{
			PRINT(LOG_INFO," client Ptr is NULL for conn_id "<<conn_id<<".exiting ");
			exit(1);
		}
		else
		{

			conn_argv.conn_id=a_client->conn_id;
			conn_argv.seq_no=a_client->seq_no;
			pckt_fmt pkt;
			pkt.conn_id=a_client->conn_id;
			pkt.seq_no=a_client->seq_no;
			pkt.data=(char*)malloc(lth +1);
			memcpy(pkt.data,pld,lth +1);
			//PRINT_PACKET(pkt,"SEND")
			PRINT(LOG_DEBUG," Lsp_server_write "<<a_client->conn_id<<" conn state "<< (a_client->conn_state) << " data sent flag "<<a_client->first_data_sent <<" ack status "<<get_ack_status(a_srv,a_client,conn_argv)<<" prev seq_no :"<<a_client->seq_no<<"\n");
			if((a_client->conn_state == CONN_REQ_ACK_SENT) && ((get_ack_status(a_srv,a_client,conn_argv)==true || a_client->first_data_sent ==false )))
			{
				server_send(a_srv,DATA_PCKT,a_client->conn_id,a_client->seq_no,(char*)pld,lth);
				if(a_client->last_pckt_sent.data != NULL)
				{
					free(a_client->last_pckt_sent.data);
				}
				a_client->last_pckt_sent.data=(char*)malloc(lth + 1);
				a_client->last_pckt_sent.conn_id=a_client->conn_id;
				a_client->last_pckt_sent.seq_no=a_client->seq_no;
				memcpy(a_client->last_pckt_sent.data,pld,lth + 1 );
				free(pkt.data);

			}
			else
			{
				PRINT(LOG_DEBUG," Ack not recieved for earlier seq no "<< a_client->seq_no <<" .Now, outbox size is "<<a_client->outbox_queue.size()<<"\n");
				outbox_struct outbox;
				outbox.pkt=pkt;
				outbox.payload_size=lth;
				a_client->outbox_queue.putq(outbox);

			}
		}
	}
}

bool lsp_server_close(lsp_server* a_srv, uint32_t conn_id)
{
 a_srv->client_conn_info.erase(conn_id);
 //close(a_srv->socket_fd);
}

void server_send(lsp_server* server,pckt_type pkt_type,int client_conn_id,int seq_no,const char *payload,int length)
{
	int numbytes;
	uint8_t* buff;
	int len =0 ;
	char* s="NIL";
	client_info* client_conn=server->client_conn_info[client_conn_id];
	PRINT(LOG_DEBUG," In server send "<<client_conn->conn_id<<" \n");
	pckt_fmt pkt;
	pkt.conn_id=client_conn_id;
	conn_arg conn_argv;
	conn_argv.conn_id=client_conn_id;
	if(pkt_type== DATA_PCKT)
	{

                client_conn->first_data_sent =true;
		client_conn->seq_no++;
		buff=message_encode(client_conn->conn_id,client_conn->seq_no,payload,len);
	        conn_argv.seq_no=client_conn->seq_no;
		pkt.seq_no=client_conn->seq_no;
		pkt.data=(char*)payload;
       	//	(client_conn->conn_map)[conn_argv]=false;
	        set_ack_status(server,client_conn,conn_argv,false);

	}
	else if(pkt_type == CONN_ACK)
	{
		buff=message_encode(client_conn->conn_id,0,"\0",len);
		conn_argv.seq_no=0;
		pkt.seq_no=0;
		pkt.data=s;

	}
	else if(pkt_type == DATA_ACK)
	{
	        buff = message_encode(client_conn->conn_id,seq_no,"\0",len);
		conn_argv.seq_no=seq_no;
		pkt.seq_no=seq_no;
		pkt.data=s;


	}
	else if(pkt_type == DATA_PCKT_RESEND)
	{
		buff=message_encode(client_conn->conn_id,seq_no,payload,len);
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
	
	if ((numbytes = sendto(server->socket_fd, buff, len, 0,
					(struct sockaddr *)&client_conn->addr, sizeof(client_conn->addr))) == -1) {
		perror("client: sendto");
		exit(1);
	}

	PRINT(LOG_DEBUG,"sent "<< numbytes<<" bytes of pkt_type "<<pkt_type<<"\n");
	free(buff);
}



bool get_ack_status(lsp_server* server,client_info* client,conn_arg arg)
{

	bool value= (client->conn_map)[arg];
	return value;
}
void set_ack_status(lsp_server* server,client_info* client,conn_arg arg, bool value)
{
	
	(client->conn_map)[arg]=value;
}




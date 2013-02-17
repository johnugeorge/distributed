#include"lsp.h"
#include"network_handler.h"
using namespace std;

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int
socket_cmp_addr(const struct sockaddr *sa1, const struct sockaddr *sa2)
{
	if (sa1->sa_family != sa2->sa_family)
	{
		PRINT(LOG_CRIT," Error in family \n");
		exit(1);
	}

	switch (sa1->sa_family) {
		case AF_INET: {
				      return(memcmp( &((struct sockaddr_in *) sa1)->sin_addr,
							      &((struct sockaddr_in *) sa2)->sin_addr,
							      sizeof(struct in_addr)));
			      }
		case AF_INET6: {
				       return(memcmp( &((struct sockaddr_in6 *) sa1)->sin6_addr,
							       &((struct sockaddr_in6 *) sa2)->sin6_addr,
							       sizeof(struct in6_addr)));
			       }
		default:
			       {
				       PRINT(LOG_CRIT," Error in family \n");
				       exit(1);
			       }
	}
}
	
int
socket_cmp_port(const struct sockaddr *sa1, const struct sockaddr *sa2)
{
	if (sa1->sa_family != sa2->sa_family)
		return(-1);
	switch (sa1->sa_family) {
		case AF_INET: {
				      return( ((struct sockaddr_in *) sa1)->sin_port ==
						      ((struct sockaddr_in *) sa2)->sin_port);
			      }

		case AF_INET6: {
				       return( ((struct sockaddr_in6 *) sa1)->sin6_port ==
						       ((struct sockaddr_in6 *) sa2)->sin6_port);
			       }

	}
	return (-1);
}
void c_network_handler(void* p)
{
  thread_info_map[pthread_self()]=" CLIENT NETWORK HANDLER THREAD :";
  PRINT(LOG_DEBUG," c_network_handler \n");
  lsp_client* client=(lsp_client*)p;
  struct sockaddr_storage their_addr;
  uint8_t raw_buf[MAX_PAYLOAD_SIZE],decoded_buf[MAX_PAYLOAD_SIZE];
  socklen_t addr_len;
  int numbytes;
  char s[INET6_ADDRSTRLEN];
  pckt_fmt pkt;
  int pkt_save=0;
  while(1)
  {
	  pkt_save=0;
	  addr_len = sizeof their_addr;
	  PRINT(LOG_DEBUG,"Waitng in client recv\n");
	  if ((numbytes = recvfrom(client->socket_fd, raw_buf, sizeof(raw_buf), 0,
					  (struct sockaddr *)&their_addr, &addr_len)) == -1) {
		  perror("recvfrom");
		  exit(1);
	  }

	  PRINT(LOG_DEBUG,"client: got packet from " <<
			  inet_ntop(their_addr.ss_family,
				  get_in_addr((struct sockaddr *)&their_addr),
				  s, sizeof s)<<" numbytes "<<numbytes<<" \n");

	
  	  pkt.data=(char*)malloc(numbytes);
	  int len=message_decode(numbytes,raw_buf,pkt);
	  PRINT_PACKET(pkt,"RECEIVE")
	  //PRINT(LOG_DEBUG,"Num bytes "<<numbytes<<" len "<<len<<"\n";
	  conn_arg conn_argv;
	  conn_argv.conn_id=client->conn_id;
	  conn_argv.seq_no=pkt.seq_no;
	 
	  {
		  PRINT(LOG_DEBUG,"Resetting epoch timers\n");
		  client->no_epochs_elapsed=0;
	  } 
	  double rand_value = (double(rand() %10)/10);
	  if(rand_value < lsp_get_drop_rate())
	  {
		  PRINT(LOG_DEBUG,"PACKET DROPPED_______________ rand value"<<rand_value<<"\n");
		  free(pkt.data);
		  continue;
	  }
	  if(strlen(pkt.data)==0)
	  {
		 
		  if((pkt.conn_id != 0 ) && (pkt.seq_no == 0))//conn_resp
		  {
			  client->conn_id= pkt.conn_id;
			  client->conn_state=CONN_REQ_ACK_RCVD;
			  PRINT(LOG_INFO," New Connection Id of the Client "<<client->conn_id<<"\n");
		  }
		  else if ((pkt.conn_id != 0 ) && (pkt.seq_no != 0))//data_ack
		  {

	          }
		  else
	          {
			  PRINT(LOG_CRIT,"Error in packet parsing\n");
		          exit(1);
		  }
		  if(get_ack_status(client,conn_argv)==true)
		  {
			  PRINT(LOG_DEBUG,"Error:Ack already recvd:Duplicate \n");
	          }
		  else
	          {
	          //(client->conn_map)[conn_argv]=true;   
		  set_ack_status(client,conn_argv,true); 
	          }	  
	  }
          else //data packet
	  {

                if(client->conn_id != pkt.conn_id)
		{
			PRINT(LOG_DEBUG," Rcvd Data packet from unknown host \n");
		}
		else
		{
			if(pkt.seq_no <= client->last_seq_no_rcvd)
			{
				PRINT(LOG_DEBUG," Duplicate or older msg Rcvd \n");
			}
			else
			{
				pkt_save=1;
				client->first_data_rcvd=true;
				client->last_seq_no_rcvd=pkt.seq_no;
				inbox_struct inbox;
				inbox.pkt=pkt;
				inbox.payload_size=len;
				client->inbox_queue.putq(inbox);
				client_send(client,DATA_ACK,pkt.seq_no);
			}
		}

          }
	if(pkt_save!= 1)
		free(pkt.data);
  }
}

void s_network_handler(void* p)
{
  thread_info_map[pthread_self()]=" SERVER NETWORK HANDLER THREAD:";
  PRINT(LOG_DEBUG," s_network_handler \n");
  lsp_server* server=(lsp_server*)p;
  struct sockaddr_storage their_addr;
  uint8_t raw_buf[MAX_PAYLOAD_SIZE],decoded_buf[MAX_PAYLOAD_SIZE];
  socklen_t addr_len;
  int numbytes;
  char s[INET6_ADDRSTRLEN];
  pckt_fmt pkt;
  int dup_req=0;
  int pkt_save=0;
  while(1)
  {
	  pkt_save=0;
	  addr_len = sizeof their_addr;
	  PRINT(LOG_DEBUG,"Waitng in server recv\n");
	  if ((numbytes = recvfrom(server->socket_fd, raw_buf, sizeof(raw_buf), 0,
					  (struct sockaddr *)&their_addr, &addr_len)) == -1) {
		  perror("recvfrom");
		  exit(1);
	  }

	  PRINT(LOG_DEBUG,"listener: got packet from "<<
			  inet_ntop(their_addr.ss_family,
				  get_in_addr((struct sockaddr *)&their_addr),
				  s, sizeof s)<<" numbytes "<<numbytes<<"\n");
	  
  	  pkt.data=(char*)malloc(numbytes);
	  int len=message_decode(numbytes,raw_buf,pkt);
	  PRINT_PACKET(pkt,"RECEIVE")
	  //PRINT(LOG_DEBUG,"Num bytes "<<numbytes<<" len "<<len<<" pkt data size "<<strlen(pkt.data)<<"\n";
	  conn_arg conn_argv;
	  //conn_argv.conn_id=client->conn_id;
	  //conn_argv.seq_no=pkt.seq_no;

	  double rand_value = (double(rand() %10)/10);
	  if(rand_value < lsp_get_drop_rate())
	  {
		  PRINT(LOG_DEBUG,"PACKET DROPPED_______________ rand value"<<rand_value<<"\n");
		  free(pkt.data);
		  continue;
	  }

	  if(strlen(pkt.data)==0)
	  {
		 
		  if((pkt.conn_id == 0 ) && (pkt.seq_no == 0))//conn_request
		  {
			  client_info_map::iterator it= server->client_conn_info.begin();
			  for(;it !=server->client_conn_info.end();it++)
			  {
				  if(socket_cmp_addr((struct sockaddr *)&their_addr,(struct sockaddr *)&(it->second->addr)) == 0)
				  {
					  if(socket_cmp_port((struct sockaddr *)&their_addr,(struct sockaddr *)&(it->second->addr)) == 1)
					  {
						  dup_req=1;
						  PRINT(LOG_DEBUG," Duplicate Client Connection Request \n");
					  }
				  }
			  }
			  if(!dup_req)
			  {
				  PRINT(LOG_DEBUG," New connection Request \n");
				  client_info* new_client  =new client_info;
				  new_client->conn_id=server->next_free_conn_id++;
				  new_client->addr=their_addr;
				  new_client->conn_state=CONN_REQ_ACK_SENT;
				  server->client_conn_info[new_client->conn_id]  =  new_client;
				  server_send(server,CONN_ACK,new_client->conn_id);
			  }
			  dup_req=0;
		  }
		  else if ((pkt.conn_id != 0 ) && (pkt.seq_no != 0))//data_ack
		  {
			  if(server->client_conn_info.find(pkt.conn_id) != server->client_conn_info.end())
			  {
				  conn_argv.conn_id=pkt.conn_id;
				  conn_argv.seq_no=pkt.seq_no;
				  client_info* client_conn=server->client_conn_info[pkt.conn_id];
				  client_conn->no_epochs_elapsed=0;
				  if(get_ack_status(server,client_conn,conn_argv)==true)
				  {
					  PRINT(LOG_DEBUG,"Error:Ack already recvd:Duplicate \n");
				  }
				  else
				  {
					  //(client_conn->conn_map)[conn_argv]=true;     
					  set_ack_status(server,client_conn,conn_argv,true) ;
				  }
			  }
			  else
			  {
				  PRINT(LOG_DEBUG,"Getting data ack from unknown client conn_id "<<pkt.conn_id);
			  }
		  }
		  else if((pkt.conn_id != 0 ) && (pkt.seq_no == 0))//client might not have reccived any data msg
		  {
			  if(server->client_conn_info.find(pkt.conn_id) != server->client_conn_info.end())
			  {
				  client_info* client_conn=server->client_conn_info[pkt.conn_id];
				  if(client_conn->last_pckt_sent.data == NULL)
				  {
					  PRINT(LOG_DEBUG,"Resetting Epoch Timers\n");
					  client_conn->no_epochs_elapsed=0;
				  }
			  }
		  }
		  else
		  {
			  PRINT(LOG_CRIT,"Unknown Packet\n");
			  //exit(1);
		  }

		  
	  
	  }
	  else //data packet
	  {
		  if(server->client_conn_info.find(pkt.conn_id) != server->client_conn_info.end())
		  {

			  client_info* client_conn=server->client_conn_info[pkt.conn_id];

			  client_conn->first_data_rcvd=true;
			  client_conn->no_epochs_elapsed=0;
			  if(client_conn->conn_id != pkt.conn_id)
			  {
				  PRINT(LOG_DEBUG," Rcvd Data packet from unknown host \n");
			  }
			  else
			  {
				  if(pkt.seq_no <= client_conn->last_seq_no_rcvd)
				  {
					  PRINT(LOG_DEBUG," Duplicate or older msg Rcvd \n");
				  }
				  else
				  {
					  pkt_save=1;
					  client_conn->last_seq_no_rcvd=pkt.seq_no;
					  //client_conn->inbox_queue.push(pkt);
					  inbox_struct inbox;
					  inbox.pkt=pkt;
					  inbox.payload_size=len;
					  server->inbox_queue.putq(inbox);
					  PRINT(LOG_DEBUG," Adding to inbox.Size is "<<server->inbox_queue.size());
					  server_send(server,DATA_ACK,client_conn->conn_id,pkt.seq_no);
				  }
			  }
		  }
		  else
		  {
			  PRINT(LOG_DEBUG,"Getting data ack from unknown client conn_id "<<pkt.conn_id);
		  }

          }
	if(pkt_save != 1)
		free(pkt.data);
  }
}


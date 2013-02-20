#include "lsp_client.h"
#include"network_handler.h"
using namespace std;

extern pthread_mutex_t global_mutex;
extern pthread_cond_t global_created;

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
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
			  pthread_mutex_lock(&global_mutex);
			  if(client->conn_state == CONN_REQ_SENT)
			  {
				  pthread_cond_signal(&global_created);
			  }
			  client->conn_state=CONN_REQ_ACK_RCVD;
			  pthread_mutex_unlock(&global_mutex);

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




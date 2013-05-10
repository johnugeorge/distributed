#include"epoch_handler.h"
using namespace std;

/*Timeout is implemented using select system call and it is run in infinite loop*/
void c_epoch_timer(void* p)
{
  thread_info_map[pthread_self()]=" CLIENT EPOCH HANDLER THREAD   :";
  struct timeval tv;
  int rv;
  char errorbuffer[256];
  lsp_client* client=(lsp_client*)p;
  while(1)
  {
    tv.tv_sec = lsp_get_epoch_lth();
    tv.tv_usec = 0;
    rv = select(0, NULL, NULL, NULL, &tv);
    
    if(rv == -1)
    {
      fprintf(stderr, "Select error: \n");
      char* errorMessage = strerror_r(errno, errorbuffer, 256);
      PRINT(LOG_CRIT,"Error: "<<errorMessage);
      exit(1);
    }
    else if(rv == 0)
    {
      //timeout occurred, handle it
      c_handle_epoch(client);
    }
  }
}

void c_handle_epoch(lsp_client* client)
{
  PRINT(LOG_DEBUG,"Client epoch timer"<<std::endl);
  if(client->closed == true)return;
  int count=client->no_epochs_elapsed;
  count++;
  if(count == lsp_get_epoch_cnt())
  {
	  PRINT(LOG_DEBUG,"Client epoch count reached. As no messages are received from server for "<<lsp_get_epoch_cnt()*lsp_get_epoch_lth()<<"s ,Shutting down"<<std::endl);
	  inbox_struct inbox;
	  inbox.payload_size=0;
	  client->inbox_queue.putq(inbox);
//	exit(0);

  }
  client->no_epochs_elapsed=count;
   
  if(client->conn_state == CONN_REQ_SENT)
  {
	  PRINT(LOG_DEBUG,"Client epoch timer:: Resending CONN_REQ"<<std::endl); 
	  client_send(client,CONN_REQ);

  }
  if((client->conn_state == CONN_REQ_ACK_RCVD) && !(client->last_seq_no_rcvd))//sending ack
  {
	  PRINT(LOG_DEBUG,"Client epoch timer:: No data Messages have been rcvd. Hence sending ACK with seq_no 0\n");
	  client_send(client,DATA_ACK,0);
  }
  else
  {
	  if((client->conn_state == CONN_REQ_ACK_RCVD)&& (client->last_seq_no_rcvd))//resending data ack
	  {
		  PRINT(LOG_DEBUG,"Client epoch timer:: Resending DATA_ACK for seq_no "<<client->last_seq_no_rcvd<<std::endl); 
		  client_send(client,DATA_ACK,client->last_seq_no_rcvd);
	  }
  }
  conn_arg conn_argv;
  conn_argv.conn_id=client->conn_id;
  conn_argv.seq_no=client->last_pckt_sent.seq_no;
  int lth=0;
  if((client->first_data_sent == true)&& (get_ack_status(client,conn_argv)!=true)) //resending data packet
  {
	  PRINT(LOG_DEBUG,"Client epoch timer:: Resending DATA_PCKT for seq_no "<<client->last_pckt_sent.seq_no<<std::endl); 
	  client_send(client,DATA_PCKT_RESEND,client->seq_no,client->last_pckt_sent.data,lth);
  }
  else
  {
	  if(!client->outbox_queue.empty())	//sending the packet which was put in the queue
	  {
		  pckt_fmt pkt;
		  outbox_struct outbox;
		  client->outbox_queue.getq(outbox);
		  pkt.data = outbox.pkt.data;
		  pkt.seq_no = outbox.pkt.seq_no;
		  lth=outbox.payload_size;
		  PRINT(LOG_DEBUG,"Client epoch timer:: Sending deferred DATA_PCKT for seq_no "<<pkt.seq_no<<std::endl); 
		  client_send(client,DATA_PCKT,client->seq_no,pkt.data,lth);
		  if(client->last_pckt_sent.data != NULL)
		  {
			  free(client->last_pckt_sent.data);
		  }
		  client->last_pckt_sent.data=(char*)malloc(lth+1);
		  client->last_pckt_sent.conn_id=client->conn_id;
		  client->last_pckt_sent.seq_no=client->seq_no;
		  memcpy(client->last_pckt_sent.data,pkt.data,lth +1);
		  free(pkt.data);
	  }
  }	
} 




#include"epoch_handler.h"
using namespace std;

void c_epoch_timer(void* p)
{
  struct timeval tv;
  int rv;
  char errorbuffer[256];
  lsp_client* client=(lsp_client*)p;
  //tv.tv_sec = 3;
  //tv.tv_sec = lsp_get_epoch_lth();
  //tv.tv_usec = 0;
  //rv = select(n, &readfds, NULL, NULL, &tv);
  while(1)
  {
//    tv.tv_sec = 3;
    tv.tv_sec = lsp_get_epoch_lth();
    tv.tv_usec = 0;
    rv = select(0, NULL, NULL, NULL, &tv);
    
    if(rv == -1)
    {
      fprintf(stderr, "Select error: \n");
      char* errorMessage = strerror_r(errno, errorbuffer, 256);
      cout<<"Error: "<<errorMessage;
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
  std::cout<<"Client epoch timer"<<std::endl; 
  if(client->conn_state == CONN_REQ_SENT)
  {
    std::cout<<"Client epoch timer:: Resending CONN_REQ"<<std::endl; 
    client_send(client,CONN_REQ);
  }
  else
  {
	if(client->last_seq_no_rcvd)
	{
        std::cout<<"Client epoch timer:: Resending DATA_ACK for seq_no "<<client->last_seq_no_rcvd<<std::endl; 
	client_send(client,DATA_ACK,client->last_seq_no_rcvd);
	}
	conn_arg conn_argv;
	conn_argv.conn_id=client->conn_id;
	conn_argv.seq_no=client->last_pckt_sent.seq_no;
	int lth=0;
        if(((client->conn_map)[conn_argv]!=true) && (client->first_data_sent == true))
	{
          std::cout<<"Client epoch timer:: Resending DATA_PCKT for seq_no "<<client->last_pckt_sent.seq_no<<std::endl; 
          client_send(client,DATA_PCKT_RESEND,client->seq_no,client->last_pckt_sent.data,lth);
	}
        else
	{
	   if(!client->outbox_queue.empty())	
	   {
	   pckt_fmt pkt;
	   client->outbox_queue.getq(pkt);
           std::cout<<"Client epoch timer:: Sending deferred DATA_PCKT for seq_no "<<pkt.seq_no<<std::endl; 
	   client_send(client,DATA_PCKT,client->seq_no,client->last_pckt_sent.data,lth);
	   }
	}	
  } 


}

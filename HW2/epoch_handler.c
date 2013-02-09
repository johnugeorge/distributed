#include"epoch_handler.h"
using namespace std;

void c_epoch_timer(void* p)
{
  thread_info_map[pthread_self()]=" CLIENT EPOCH HANDLER THREAD   :";
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
        if((get_ack_status(client,conn_argv)!=true) && (client->first_data_sent == true))
	{
          std::cout<<"Client epoch timer:: Resending DATA_PCKT for seq_no "<<client->last_pckt_sent.seq_no<<std::endl; 
          client_send(client,DATA_PCKT_RESEND,client->seq_no,client->last_pckt_sent.data,lth);
	}
        else
	{
	   if(!client->outbox_queue.empty())	
	   {
	   pckt_fmt pkt;
	   outbox_struct outbox;
	   client->outbox_queue.getq(outbox);
	   pkt.data = outbox.pkt.data;
	   pkt.seq_no = outbox.pkt.seq_no;
	   lth=outbox.payload_size;
           std::cout<<"Client epoch timer:: Sending deferred DATA_PCKT for seq_no "<<pkt.seq_no<<std::endl; 
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


}

void s_epoch_timer(void* p)
{
  thread_info_map[pthread_self()]=" SERVER EPOCH HANDLER THREAD   :";
  struct timeval tv;
  int rv;
  char errorbuffer[256];
  lsp_server* server=(lsp_server*)p;
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
      s_handle_epoch(server);
    }
  }
}

void s_handle_epoch(lsp_server* server)
{
	std::cout<<"Server epoch timer"<<std::endl;
        client_info* client;	
	client_info_map::iterator it= server->client_conn_info.begin();
	for(;it !=server->client_conn_info.end();it++)
	{
		client=it->second;
		if((client->conn_state == CONN_REQ_ACK_SENT) && !((client->last_seq_no_rcvd)))
		{
			std::cout<<"Server epoch timer:: Resending CONN_ACK for conn_id "<<client->conn_id<<std::endl; 
			server_send(server,CONN_ACK,client->conn_id);
		}
		else
		{
			if(client->last_seq_no_rcvd)
			{
				std::cout<<"Server epoch timer:: Resending DATA_ACK for seq_no "<<client->last_seq_no_rcvd<<" conn_id "<<client->conn_id<<std::endl; 
				server_send(server,DATA_ACK,client->conn_id,client->last_seq_no_rcvd);
			}
			conn_arg conn_argv;
			conn_argv.conn_id=client->conn_id;
			conn_argv.seq_no=client->last_pckt_sent.seq_no;
			int lth=0;
			if((get_ack_status(server,client,conn_argv)!=true) && (client->first_data_sent == true))
			{
				std::cout<<"Server epoch timer:: Resending DATA_PCKT for seq_no "<<client->last_pckt_sent.seq_no<<" conn_id "<<client->conn_id<<std::endl; 
				server_send(server,DATA_PCKT_RESEND,client->conn_id,client->seq_no,client->last_pckt_sent.data,lth);
			}
			else
			{
				if(!client->outbox_queue.empty())	
				{
					pckt_fmt pkt;
	   				outbox_struct outbox;
					client->outbox_queue.getq(outbox);
					pkt.data = outbox.pkt.data;
					pkt.seq_no = outbox.pkt.seq_no;
					lth=outbox.payload_size;

					std::cout<<"Server epoch timer:: Sending deferred DATA_PCKT for seq_no "<<pkt.seq_no<<std::endl; 
					server_send(server,DATA_PCKT,client->conn_id,client->seq_no,pkt.data,lth);
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

	}
}

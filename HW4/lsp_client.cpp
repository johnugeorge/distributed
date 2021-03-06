#include "lsp_client.h"
#include <rpc/pmap_clnt.h>
#include<rpc/rpc.h>
double epoch_delay = _EPOCH_LTH; // number of seconds between epochs
unsigned int num_epochs = _EPOCH_CNT; // number of epochs that are allowed to pass before a connection is terminated

/*
 *
 *
 *				LSP RELATED FUNCTIONS
 *
 *
 */  
lsp_client* sClient;

// Set length of epoch (in seconds)
void lsp_set_epoch_lth(double lth){
    if(lth > 0)
        epoch_delay = lth;
}

// Set number of epochs before timing out
void lsp_set_epoch_cnt(int cnt){
    if(cnt > 0)
        num_epochs = cnt;
}

// Set fraction of packets that get dropped along each connection
void lsp_set_drop_rate(double rate){
    network_set_drop_rate(rate);
}

int * recvfn1(LSPMessage1 * msg)
{
	if(DEBUG) printf(" Incoming Packet size of Payload %d \n", strlen(msg->payload));
	static int ret= 0;
	int conn_id=msg->connid;
	int seq_no=msg->seqnum;
	std::string payload=msg->payload;
	LSPMessage *pkt = new LSPMessage;
	pkt->conn_id=conn_id;
	pkt->seq_num=seq_no;
	pkt->data=payload;
	sClient->rpcInbox.putq(pkt);
	return &ret;

}


void  callback(struct svc_req *rqstp, SVCXPRT * transp)
{
	union {
		LSPMessage1 test_func_1_arg;
	} argument;


	switch (rqstp->rq_proc) {
		case 0:
			if (!svc_sendreply(transp, (xdrproc_t) xdr_void, 0)) {
				fprintf(stderr, "err: exampleprog\n");
				//return (1);
				return ;
			}
			//return (0);
			return ;
		case 1:

			memset ((char *)&argument, 0, sizeof (argument));

			if (!svc_getargs(transp,  (xdrproc_t) xdr_LSPMessage1, (caddr_t) &argument)) {
				svcerr_decode(transp);
				//return (1);
				return ;
			}
			//printf(" Successful decoding %s %d\n " ,test_func_1_arg.name, argument.val);
			int* result =recvfn1((LSPMessage1 *)&argument);
			if(DEBUG) fprintf(stderr, "client got callback\n");
			if (!svc_sendreply(transp, (xdrproc_t) (xdrproc_t) xdr_int,(caddr_t) result)) {
				fprintf(stderr, "err: exampleprog");
				//return (1);
				return ;
			}
	}
}
	//																						}
/*
 *
 *
 *				CLIENT RELATED FUNCTIONS
 *
 *
 */  

lsp_client* lsp_client_create(const char* dest, int port){
    lsp_client *client = new lsp_client();
    sClient=client;
    pthread_mutex_init(&(client->mutex),NULL);
    pthread_mutex_init(&(client->rpcMutex),NULL);
    client->connection = network_make_connection(dest,port);
    
    if(!client->connection){
        // connection could not be made
        lsp_client_close(client);
        return NULL;
    }
   
    CLIENT *clnt;
    char **result; /* return value */

    clnt = clnt_create(dest, LSP_PROGRAM, LSP_VERS, "udp");
    if (clnt == NULL) {
	    clnt_pcreateerror(dest);
	    exit(1);
    }

    client->callback=0;
    client->connection->lastSentSeq = 0;
    client->connection->lastReceivedSeq = 0;
    client->connection->lastReceivedAck = 0;
    client->connection->epochsSinceLastMessage = 0;
    client->clnt_handle=clnt; 
    // kickoff new epoch timer
    int res;
    if((res = pthread_create(&(client->epochThread), NULL, ClientEpochThread, (void*)client)) != 0){
        printf("Error: Failed to start the epoch thread: %d\n",res);
        lsp_client_close(client);
        return NULL;
    }

/*    if(network_send_connection_request(client->connection) && 
       network_wait_for_connection(client->connection, epoch_delay * num_epochs))
       */
       {

	
        pthread_mutex_lock(&(client->mutex));
        client->connection->host = dest;
	
       	if((res = pthread_create(&(client->RPCThread), NULL, ClientRPCThread, (void*)client)) != 0){
            printf("Error: Failed to start the write thread: %d\n",res);
            lsp_client_close(client);
            return NULL;
        } 

      	while(client->callback==0); 

	if(rpc_wait_for_connection(client))
	{
		//LSPMessage *msg = network_build_message(0,0,(uint8_t*)"\0",0);
		//client_send_message(client, msg);

		// connection succeeded, build lsp_client struct        
		client->connection->port = port;
		client->connection->status = CONNECTED;

		// kick off ReadThread to catch incoming messages
		int res;
		if((res = pthread_create(&(client->readThread), NULL, ClientReadThread, (void*)client)) != 0){
			printf("Error: Failed to start the read thread: %d\n",res);
			lsp_client_close(client);
			return NULL;
		}
		if((res = pthread_create(&(client->writeThread), NULL, ClientWriteThread, (void*)client)) != 0){
			printf("Error: Failed to start the write thread: %d\n",res);
			lsp_client_close(client);
			return NULL;
		}

		pthread_mutex_unlock(&(client->mutex));
		return client;
	}
	else {
		//	 connection failed or timeout after K * delta seconds
		pthread_mutex_unlock(&(client->mutex));
		lsp_client_close(client);
		return NULL;
	}
    }/* else {
        // conneed or timeout after K * delta seconds
        lsp_client_close(client);
        return NULL;
    }*/
}

void client_acknowledge(lsp_client* client)
{
   Connection* conn = client->connection;
    LSPMessage *msg = network_build_message(conn->id, conn->lastReceivedSeq, NULL, 0);

   send_message(client, msg);
}

void send_message(lsp_client* client, LSPMessage* msg)
{
  client_send_message(client, msg);
}


void client_send_message(lsp_client* client,LSPMessage* msg)
{
	if(DEBUG)printf(" client_send_message start %s \n",client->connection->host);
	LSPMessage1 pkt;
        pkt.connid=msg->conn_id;
	pkt.seqnum=msg->seq_num;
	pkt.payload=(char*)(msg->data).c_str();
        pthread_mutex_lock(&(client->rpcMutex));
	int result;
	int ans = callrpc(client->connection->host, LSP_PROGRAM, LSP_VERS,
			(__const u_long) 1, (__const xdrproc_t) xdr_LSPMessage1, (char*)&pkt, (__const xdrproc_t) xdr_int, (char*)&result);

	/*if ((enum clnt_stat) ans != RPC_SUCCESS) {
		fprintf(stderr, "call callrpc: Send Msg ");
		clnt_perrno((enum clnt_stat)ans);
		fprintf(stderr, "\n");
	}*/

	if(DEBUG)printf(" client_send_message Conn Id %d end\n",result);
        pthread_mutex_unlock(&(client->rpcMutex));
	client->connection->id=result;

}

int lsp_client_read(lsp_client* a_client, uint8_t* pld){
    // block until a message arrives or the client becomes disconnected
    while(true){
        pthread_mutex_lock(&(a_client->mutex));
        Status s = a_client->connection->status;
        LSPMessage *msg = NULL;
        if(s == CONNECTED) {
            // try to pop a message off of the inbox queue
            if(a_client->inbox.size() > 0){
                msg = a_client->inbox.front();
                a_client->inbox.pop();
            }
        }
        pthread_mutex_unlock(&(a_client->mutex));
        if(s == DISCONNECTED)
            break;
           
        // we got a message, so return it
        if(msg){
            std::string payload = msg->payload();
            delete msg;
            memcpy(pld,payload.c_str(),payload.length()+1);
            return payload.length();
        }
        
        // still connected, but no message has arrived...
        // sleep for a bit
        usleep(10000); // 10 ms = 10,0000 microseconds
    }
    if(DEBUG) printf("Client was disconnected. Read returning NULL\n");
    return 0; // NULL, no bytes read (client disconnected)
}

bool lsp_client_write(lsp_client* a_client, uint8_t* pld, int lth){
    // queues up a message to be written by the Write Thread
    
    if(pld == NULL || lth == 0)
        return false; // don't send bad messages
    
    pthread_mutex_lock(&(a_client->mutex));
    a_client->connection->lastSentSeq++;
    if(DEBUG) printf("Client queueing msg %d for write\n",a_client->connection->lastSentSeq);
    
    // build the message
    LSPMessage *msg = network_build_message(a_client->connection->id,a_client->connection->lastSentSeq,pld,lth);
    
    // queue it up
    a_client->connection->outbox.push(msg);
    pthread_mutex_unlock(&(a_client->mutex));
    
    return true;
}

bool lsp_client_close(lsp_client* a_client){
    // returns true if the connected was closed,
    // false if it was already previously closed
    
    if(DEBUG) printf("Shutting down the client\n");
    
    pthread_mutex_lock(&(a_client->mutex));
    bool alreadyClosed = (a_client->connection && a_client->connection->status == DISCONNECTED);
    if(a_client->connection)
        a_client->connection->status = DISCONNECTED;
    pthread_mutex_unlock(&(a_client->mutex));
    
    cleanup_client(a_client);
    return !alreadyClosed;
}

/* Internal Methods */

void* ClientRPCThread(void *params){
	    lsp_client *client = (lsp_client*)params;
	    
	int x, ans, s;
	s = RPC_ANYSOCK;
	x = gettransient(IPPROTO_UDP, 1, &s);
	SVCXPRT *xprt;
	if ((xprt = svcudp_create(s)) == NULL) {
		fprintf(stderr, "rpc_server: svcudp_create\n");
		exit(1);
	}
	(void)svc_register(xprt, (rpcprog_t) x, 1, callback, 0);


	if(DEBUG)printf(" making RPC call %d\n",x);
	ans = callrpc(client->connection->host, LSP_PROGRAM, LSP_VERS,
			(__const u_long) 3, (__const xdrproc_t) xdr_int, (char*)&x, (__const xdrproc_t) xdr_void, 0);

	if(DEBUG)printf(" RPC called\n");
	if ((enum clnt_stat) ans != RPC_SUCCESS) {
		fprintf(stderr, "call callrpc: ");
		clnt_perrno((enum clnt_stat)ans);
		fprintf(stderr, "\n");
	}
	LSPMessage *msg = network_build_message(0,0,(uint8_t*)"\0",0);
	client_send_message(client, msg);
	client->callback=1;


	svc_run();

}


void* ClientEpochThread(void *params){
    lsp_client *client = (lsp_client*)params;
    
    while(true){
        usleep(epoch_delay * 1000000); // convert seconds to microseconds
        if(DEBUG) printf("Client epoch handler waking up \n");
        
        // epoch is happening; send required messages
        pthread_mutex_lock(&(client->mutex));
        if(client->connection->status == DISCONNECTED)
            break;
        
        if(client->connection->status == CONNECT_SENT){
            // connect sent already, but not yet acknowledged
            if(DEBUG) printf("Client resending connection request\n");
            network_send_connection_request(client->connection);
        } else if(client->connection->status == CONNECTED){
            // send ACK for most recent message
            if(DEBUG) printf("Client acknowledging last received message: %d\n",client->connection->lastReceivedSeq);
            //network_acknowledge(client->connection);
            client_acknowledge(client);
            
            // resend the first message in the outbox, if any
            if(client->connection->outbox.size() > 0) {
                if(DEBUG) printf("Client resending msg %d\n",client->connection->outbox.front()->seqnum());
                //network_send_message(client->connection,client->connection->outbox.front());
               send_message(client, client->connection->outbox.front()); 
            }
        } else {
            if(DEBUG) printf("Unexpected client status: %d\n",client->connection->status);
        }
        
        if(++(client->connection->epochsSinceLastMessage) >= num_epochs){
            // oops, we haven't heard from the server in a while;
            // mark the connection as disconnected
            if(DEBUG) printf("Too many epochs have passed since we heard from the server... disconnecting\n");
            client->connection->status = DISCONNECTED;
            break;
        }
        pthread_mutex_unlock(&(client->mutex));
    }
    pthread_mutex_unlock(&(client->mutex));
    if(DEBUG) printf("Epoch Thread exiting\n");
    return NULL;
}

void* ClientReadThread(void *params){
    lsp_client *client = (lsp_client*)params;
    
    // continuously poll for new messages and process them;
    // Exit when the client is disconnected
    while(true){
        pthread_mutex_lock(&(client->mutex));
        Status state = client->connection->status;
        pthread_mutex_unlock(&(client->mutex));
        
        if(state == DISCONNECTED)
            break;
        
        // attempt to read
        sockaddr_in addr;
        //LSPMessage *msg = network_read_message(client->connection, 0.5,&addr);
        LSPMessage* msg = rpc_read_message(client, 0.5);
        if(msg) {
	    if(DEBUG)msg->print();
            if(msg->connid() == client->connection->id){
                pthread_mutex_lock(&(client->mutex));
                
                // reset counter for epochs since we have received a message
                client->connection->epochsSinceLastMessage = 0;
                
                if(msg->payload().length() == 0){
                    // we received an ACK
                    if(DEBUG)printf("Client received an ACK for msg %d\n",msg->seqnum());
                    if(msg->seqnum() == (client->connection->lastReceivedAck + 1)){
                        // this sequence number is next in line, even if it overflows
                        client->connection->lastReceivedAck = msg->seqnum();
                    }
                    if(client->connection->outbox.size() > 0 && msg->seqnum() == client->connection->outbox.front()->seqnum()) {
                        delete client->connection->outbox.front();
                        client->connection->outbox.pop();
                    }
                } else {
                    // data packet
                    if(DEBUG) printf("Client received msg %d\n",msg->seqnum());
                    if(msg->seqnum() == (client->connection->lastReceivedSeq + 1)){
                        // next in the list
                        client->connection->lastReceivedSeq++;
                        client->inbox.push(msg);
                        
                        // send ack for this message
                        //network_acknowledge(client->connection);
                        client_acknowledge(client);
                    }
                }
                
                pthread_mutex_unlock(&(client->mutex));
            }
        }
    }
    if(DEBUG) printf("Read Thread exiting\n");
    return NULL;
}

// this write thread will ensure that messages can be sent/received faster than only
// on epoch boundaries. It will continuously poll for messages that are eligible to
// bet sent for the first time, and then send them out.
void* ClientWriteThread(void *params){
    lsp_client *client = (lsp_client*)params;
    
    // continuously poll for new messages to send;
    // Exit when the client is disconnected
    
    unsigned int lastSent = 0;
    
    while(true){
        pthread_mutex_lock(&(client->mutex));
        Status state = client->connection->status;
        
        if(state == DISCONNECTED)
            break;
            
        unsigned int nextToSend = client->connection->lastReceivedAck + 1;
        if(nextToSend > lastSent){
            // we have received an ack for the last message, and we haven't sent the
            // next one out yet, so if it exists, let's send it now
            if(client->connection->outbox.size() > 0) {
                //network_send_message(client->connection,client->connection->outbox.front());
                send_message(client, client->connection->outbox.front());
                lastSent = client->connection->outbox.front()->seqnum();
            }                
        }
        pthread_mutex_unlock(&(client->mutex));
        usleep(5000); // 5ms
    }
    pthread_mutex_unlock(&(client->mutex));
    return NULL;
}

void cleanup_client(lsp_client *client){
    // wait for threads to close
    void *status;
    if(client->readThread)
        pthread_join(client->readThread,&status);
    if(client->writeThread)
        pthread_join(client->writeThread,&status);
    if(client->epochThread)
        pthread_join(client->epochThread,&status);
    
    // cleanup the memory and connection
    pthread_mutex_destroy(&(client->mutex));
    pthread_mutex_destroy(&(client->rpcMutex));
    cleanup_connection(client->connection);
    delete client;
}

void cleanup_connection(Connection *s){
    if(!s)
        return;
    
    // close the file descriptor and free memory
    //if(s->fd != -1)
    //    close(s->fd);
    //delete s->addr;
    delete s;
}

int gettransient(int proto, int  vers, int* sockp)
{
	static int prognum = 0x40040000;
	int s, len, socktype;
	struct sockaddr_in addr;
	switch(proto) {
		case IPPROTO_UDP:
			socktype = SOCK_DGRAM;
			break;
		case IPPROTO_TCP:
			socktype = SOCK_STREAM;
			break;
		default:
			fprintf(stderr, "unknown protocol type\n");
			return 0;
	}
	if (*sockp == RPC_ANYSOCK) {
		if ((s = socket(AF_INET, socktype, 0)) < 0) {
			perror("socket");
			return (0);
		}
		*sockp = s;
	}
	else
		s = *sockp;
	addr.sin_addr.s_addr = 0;
	addr.sin_family = AF_INET;
	addr.sin_port = 0;
	len = sizeof(addr);
	/*
	 * 	 * * may be already bound, so don’t check for error
	 * 	 	 * */
	bind(s, (struct sockaddr *) &addr, len);
	if (getsockname(s, (struct sockaddr *) &addr, (socklen_t *)&len)< 0) {
		perror("getsockname");
		return (0);
	}
	while (!pmap_set(prognum++, vers, proto,
				ntohs(addr.sin_port))) continue;
	return (prognum-1);
}

LSPMessage* rpc_read_message(lsp_client* client, double timeout)
{
  timeval t = network_get_timeval(timeout);
  while(true)
  {
    int result = select(NULL, NULL, NULL, NULL, &t);

	    if(result == -1){
		    printf("Error receiving message: %s\n",strerror(errno));
		    return NULL;
	    } else if (result == 0) {
		    if(DEBUG) printf("Receive timed out after %.2f seconds\n",timeout);


		    LSPMessage* pkt= NULL;
		    if(!sClient->rpcInbox.empty())
		    {
			    sClient->rpcInbox.getq(pkt);
		    }
		    if(network_should_drop())
		    {
			    continue; // drop the packet and continue reading
		    }
		    return pkt;
	    }


  }
}

bool rpc_wait_for_connection(lsp_client *client){
	Connection* conn=client->connection;
	int retry=1;
	double timeout=1.0;
	while(retry <= num_epochs*epoch_delay)
	{
		LSPMessage *msg = rpc_read_message(client, timeout);
		if (msg && msg->seqnum() == 0){
			conn->id = msg->connid();
			printf("[%d] Connected\n",conn->id); 
			return true;
		} else {
			printf("Connecting to server \n");
			retry++;
			//return false;
		}
	}
	printf("Max Retries:: Timed out waiting for connection from server after %.2f seconds\n",epoch_delay * num_epochs);
	return false;
}



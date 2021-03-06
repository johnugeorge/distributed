#include "lsp_server.h"
#include <rpc/pmap_clnt.h>
#include <sstream>
#include <iostream>

double epoch_delay = _EPOCH_LTH; // number of seconds between epochs
unsigned int num_epochs = _EPOCH_CNT; // number of epochs that are allowed to pass before a connection is terminated
lsp_server* sLspServer;

/*
 *
 *
 *				LSP RELATED FUNCTIONS
 *
 *
 */  

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

/*
 *
 *
 *				SERVER RELATED FUNCTIONS
 *
 *
 */  

 // Create a server listening on a specified port.
// Returns NULL if server cannot be created

void *
callbackfn_1_svc(int *pnump,struct svc_req * req)
{
	int pnum = *(int *)pnump;
	rpcInbox_struct inbox;
        inbox.addr=req->rq_xprt->xp_raddr.sin_addr.s_addr;
        inbox.port=req->rq_xprt->xp_raddr.sin_port;
	sLspServer->client_id_map[inbox]=pnum;
}

lsp_server* lsp_server_create(int port){
    // initialize the server data structure
    lsp_server *server = new lsp_server();
    sLspServer = server;
    server->port = port;
    server->nextConnID = 1;
    server->running = true;
    server->connection = network_setup_server(port);

    if(!server->connection){
        // the server could not be bound to the specified port
        delete server;
        return NULL;
    }
    
    // initialize the mutex
    pthread_mutex_init(&(server->mutex),NULL);
    
    // create the epoch thread
    int res;
    if((res = pthread_create(&(server->RPCThread), NULL, RPCThread, (void*)server)) != 0){
        printf("Error: Failed to start the write thread: %d\n",res);
        lsp_server_close(server,0);
        return NULL;
    }
    if((res = pthread_create(&(server->epochThread), NULL, ServerEpochThread, (void*)server)) != 0){
        printf("Error: Failed to start the epoch thread: %d\n",res);
        lsp_server_close(server,0);
        return NULL;
    }
       
    // create the read/write threads listening on a certain port
    if((res = pthread_create(&(server->readThread), NULL, ServerReadThread, (void*)server)) != 0){
        printf("Error: Failed to start the epoch thread: %d\n",res);
        lsp_server_close(server,0);
        return NULL;
    } 
    if((res = pthread_create(&(server->writeThread), NULL, ServerWriteThread, (void*)server)) != 0){
        printf("Error: Failed to start the write thread: %d\n",res);
        lsp_server_close(server,0);
        return NULL;
    }
    
    return server;
}

// Read from connection. Return NULL when connection lost
// Returns number of bytes read. conn_id is an output parameter
int lsp_server_read(lsp_server* a_srv, void* pld, uint32_t* conn_id){
    // block until a message arrives or the client becomes disconnected
    while(true){
        pthread_mutex_lock(&(a_srv->mutex));
        bool running = a_srv->running;
        LSPMessage *msg = NULL;
        if(running) {
            // try to pop a message from the inbox queue
            if(a_srv->inbox.size() > 0){
                msg = a_srv->inbox.front();
                a_srv->inbox.pop();
            }
        }
        pthread_mutex_unlock(&(a_srv->mutex));
        if(!running)
            break;
        if(msg){
            // we got a message, return it
            std::string payload = msg->payload();
            *conn_id = msg->connid();
            delete msg;
            memcpy(pld,payload.c_str(),payload.length()+1);
            return payload.length();
        }
        
        // still connected, but no message has arrived...
        // sleep for a bit
        usleep(10000); // 10 ms = 10,0000 microseconds
    }
    *conn_id = 0; // all clients are disconnected
    return 0; // NULL, no bytes read (all clients disconnected)
}

// Server Write. Should not send NULL
bool lsp_server_write(lsp_server* a_srv, void* pld, int lth, uint32_t conn_id){
    if(pld == NULL || lth == 0)
        return false; // don't send bad messages
    
    pthread_mutex_lock(&(a_srv->mutex));
    Connection *conn = a_srv->clients[conn_id];
    conn->lastSentSeq++;
    if(DEBUG) printf("Server queueing msg %d for conn %d for write\n",conn->lastSentSeq,conn->id);
    
    // build the message object
    LSPMessage *msg = network_build_message(conn->id,conn->lastSentSeq,(uint8_t*)pld,lth);
    
    // queue it up for writing
    conn->outbox.push(msg);
    pthread_mutex_unlock(&(a_srv->mutex));
    
    return true;
}

// Close connection.
bool lsp_server_close(lsp_server* a_srv, uint32_t conn_id){
    if(conn_id == 0){
        // close all connections
        if(DEBUG) printf("Shutting down the server\n");
        
        for(std::map<unsigned int,Connection*>::iterator it = a_srv->clients.begin();
            it != a_srv->clients.end();
            ++it){
            Connection *conn = it->second;
            cleanup_connection(conn);
        }
        a_srv->clients.clear();
        a_srv->connections.clear();
        a_srv->running = false;
        
        // wait for the threads to terminate
        void *status;
        if(a_srv->readThread)
            pthread_join(a_srv->readThread,&status);
        if(a_srv->writeThread)
            pthread_join(a_srv->writeThread,&status);
        if(a_srv->epochThread)
            pthread_join(a_srv->epochThread,&status);
            
        pthread_mutex_destroy(&(a_srv->mutex));
        cleanup_connection(a_srv->connection);
        delete a_srv;     
    } else {
        // close one connection
        if(DEBUG) printf("Shutting down client %d\n",conn_id);
        Connection *conn = a_srv->clients[conn_id];
        delete conn->addr;
        delete conn;        
        a_srv->clients.erase(conn_id);
    }
    return false;
}

/* Internal Methods */
void* RPCThread(void*params)
{
	pmap_unset (LSP_PROGRAM, LSP_VERS);
	lsp_server* server=(lsp_server*)params;
	server->transp = svcudp_create(RPC_ANYSOCK);
	if (server->transp == NULL) {
		fprintf (stderr, "%s", "cannot create udp service.");
		exit(1);
	}

	if (!svc_register(server->transp, LSP_PROGRAM, LSP_VERS, lsp_program_1, IPPROTO_UDP)) {
		fprintf (stderr, "%s", "unable to register (TEST_PROG, TEST_VERS, udp).");
		exit(1);
	}

	svc_run();
}


void* ServerEpochThread(void *params){
    lsp_server *server = (lsp_server*)params;
    
    while(true){
        usleep(epoch_delay * 1000000); // convert seconds to microseconds
        if(DEBUG) printf("Server epoch handler waking up\n");
        
        // epoch is happening; send required messages
        pthread_mutex_lock(&(server->mutex));
        if(!server->running)
            break;
            
        // iterate over all of the connections and apply the Epoch rules for each one
        for(std::map<unsigned int,Connection*>::iterator it=server->clients.begin();
            it != server->clients.end();
            ++it){
           
            Connection *conn = it->second;
            
            if(conn->status == DISCONNECTED)
                continue;
            
             // send ACK for most recent message
            if(DEBUG) printf("Server acknowledging last received message %d for conn %d\n", conn->lastReceivedSeq,conn->id);
            network_acknowledge(conn);
            //rpc_ack(conn);
            
            // resend the first message in the outbox, if any
            if(conn->outbox.size() > 0) {
                if(DEBUG) printf("Server resending msg %d for conn %d\n",conn->outbox.front()->seqnum(),conn->id);
                network_send_message(conn,conn->outbox.front());
            }
            
            if(++(conn->epochsSinceLastMessage) >= num_epochs){
                // oops, we haven't heard from the client in a while;
                // mark the connection as disconnected
                if(DEBUG) printf("Too many epochs have passed since we heard from client %d... disconnecting\n",conn->id);
                conn->status = DISCONNECTED;
                
                // place a "disconnected" message in the queue to notify the client
                server->inbox.push(network_build_message(conn->id,0,NULL,0));
            }
        }
            
        pthread_mutex_unlock(&(server->mutex));
    }
    pthread_mutex_unlock(&(server->mutex));
    if(DEBUG) printf("Epoch Thread exiting\n");
    return NULL;
}

void* ServerReadThread(void *params){
    // continously attempt to read messages from the socket. When one arrives, parse it
    // and take the appropriate action
    
    lsp_server *server = (lsp_server*)params;
    //char host[128];
    unsigned long host;
    while(true){     
        pthread_mutex_lock(&(server->mutex));
        if(!server->running)
            break;       
        pthread_mutex_unlock(&(server->mutex));
        
	unsigned long  addr;
	unsigned short  port;
        LSPMessage *msg = rpc_read_message(server->connection, 0.1 ,addr,port);
	host=addr+port;
        if(msg) {
            // we got a message, let's parse it
            if(DEBUG) printf(" we got a message, let's parse it \n");
	    if(DEBUG) msg->print();
            pthread_mutex_lock(&(server->mutex));
	    if(DEBUG) std::cout<<" host "<<addr<<"  "<<server->connections.count(addr)<<"\n";
            if(msg->connid() == 0 && msg->seqnum() == 0 && msg->payload().length() == 0){
                // connection request, if first time, make the connection
                //sprintf(host,"%s:%d",inet_ntoa(addr.sin_addr),addr.sin_port);
                if(server->connections.count(host) == 0){
                    // this is the first time we've seen this host, add it to the server's list of seen hosts
                    server->connections.insert(host);
		    int client_id=0;
                    rpcInbox_struct inbox;
		    inbox.addr=addr;
		    inbox.port=port;
		    std::map<rpcInbox_struct,int> ::iterator it =server->client_id_map.find(inbox);
		    if(it != server->client_id_map.end())
		    { 
			    client_id=it->second;
		    	    if(DEBUG)printf("Client Id %d \n",client_id);
	            }
		    else
		    {
			    printf(" Error in getting address and port %d %d\n",addr,port);
			    exit(1);
		    }
                    if(DEBUG) printf("Connection request received from %d\n",host);
                    
                    // build up the new connection object
                    Connection *conn = new Connection();
                    conn->status = CONNECTED;
                    conn->id = server->nextConnID;
                    server->nextConnID++;
                    conn->lastSentSeq = 0;
                    conn->lastReceivedSeq = 0;
                    conn->epochsSinceLastMessage = 0;
                    conn->fd = server->connection->fd; // send through the server's socket
                    //conn->addr = new sockaddr_in();
		    conn->client_id=client_id;
		    conn->in_addr_int=addr;
                    //memcpy(conn->addr,&addr,sizeof(addr));
                    
                    if(DEBUG)printf("Connection ack from %d\n",host);
                    // send an ack for the connection request
                    network_acknowledge(conn);
                    
                    // insert this connection into the list of connections
                    server->clients.insert(std::pair<int,Connection*>(conn->id,conn));
                }
            } else {
                if(server->clients.count(msg->connid()) == 0){
                    printf("Bogus connection id received: %d, skipping message...\n",msg->connid());
                    printf("Bogus connection id received: %d, skipping message...\n",msg->seqnum());
                    printf("Bogus connection id received: %d, skipping message...\n",msg->payload().length());
                } else {
                    Connection *conn = server->clients[msg->connid()];
                
                    // reset counter for epochs since we have received a message
                    conn->epochsSinceLastMessage = 0;
                
                    if(msg->payload().length() == 0){
                        // we received an ACK
                        if(DEBUG) printf("Server received an ACK for conn %d msg %d\n",msg->connid(),msg->seqnum());
                        if(msg->seqnum() == (conn->lastReceivedAck + 1))
                            conn->lastReceivedAck = msg->seqnum();
                        if(conn->outbox.size() > 0 && msg->seqnum() == conn->outbox.front()->seqnum()) {
                            delete conn->outbox.front();
                            conn->outbox.pop();
                        }
                    } else {
                        // data packet
                        if(DEBUG) printf("Server received msg %d for conn %d\n",msg->seqnum(),msg->connid());
                        if(msg->seqnum() == (conn->lastReceivedSeq + 1)){
                            // next in the list
                            conn->lastReceivedSeq++;
                            server->inbox.push(msg);
                        
                            // send ack for this message
                            network_acknowledge(conn);
                            //rpc_ack(conn)
                        }
                    }
                }
            }
            pthread_mutex_unlock(&(server->mutex));    
        }
    }
    pthread_mutex_unlock(&(server->mutex));
    if(DEBUG) printf("Read Thread exiting\n");
    return NULL;
}

// this write thread will ensure that messages can be sent/received faster than only
// on epoch boundaries. It will continuously poll for messages that are eligible to
// bet sent for the first time, and then send them out.
void* ServerWriteThread(void *params){
    lsp_server *server = (lsp_server*)params;
    
    // continuously poll for new messages to send
    
    // store the last sent seq number for each client so that
    // we only send each message once
    std::map<unsigned int, unsigned int> lastSent;  
    
    while(true){     
        pthread_mutex_lock(&(server->mutex));
        if(!server->running)
            break;
            
        // iterate through all clients and see if they have messages to send
        for(std::map<unsigned int,Connection*>::iterator it=server->clients.begin();
            it != server->clients.end();
            ++it){
           
            Connection *conn = it->second;
            
            if(conn->status == DISCONNECTED)
                continue;
            
            unsigned int nextToSend = conn->lastReceivedAck + 1;
            if(nextToSend > lastSent[conn->id]){
                // we have received an ack for the last message, and we haven't sent the
                // next one out yet, so if it exists, let's send it now
                if(conn->outbox.size() > 0) {
                    network_send_message(conn,conn->outbox.front());
                    lastSent[conn->id] = conn->outbox.front()->seqnum();
                }                
            }
        }
        pthread_mutex_unlock(&(server->mutex));
        usleep(5000); // 5ms
    }
    pthread_mutex_unlock(&(server->mutex));
    return NULL;
}

void cleanup_connection(Connection *s){
    if(!s)
        return;

    // close the file descriptor and free memory
    if(s->fd != -1)
        close(s->fd);
    delete s->addr;
    delete s;
}

int * sendfn_1_svc(LSPMessage1 * msg, struct svc_req *req)
{
	if(DEBUG) printf("In sendFn \n");
	if(DEBUG) printf(" Incoming Packet Conn Id %d \n", msg->connid);
	if(DEBUG) printf(" Incoming Packet Seq No %d \n", msg->seqnum);
	if(DEBUG) printf(" Incoming Packet Payload %s\n", msg->payload);
	if(DEBUG) printf(" Incoming Packet size of Payload %d \n", strlen(msg->payload));
	static int ret;
	int conn_id=msg->connid;
	int seq_no=msg->seqnum;
	char *host= new char[INET6_ADDRSTRLEN];
	rpcInbox_struct inbox;
	std::string payload=msg->payload;
	LSPMessage *pkt = new LSPMessage;
	pkt->conn_id=conn_id;
	pkt->seq_num=seq_no;
	pkt->data=payload;
	//std::string host;
	inbox.lspmsg= pkt;

	//inet_ntop(AF_INET, &(req->rq_xprt->xp_raddr.sin_addr), host, INET6_ADDRSTRLEN);
	inbox.addr=req->rq_xprt->xp_raddr.sin_addr.s_addr;
	inbox.port=req->rq_xprt->xp_raddr.sin_port;
	//printf(" host %s \n",hostname.c_str());
	//std::cout<<" host "<<inbox.addr<<"\n";
	sLspServer->rpcInbox.putq(inbox);
	if((conn_id == 0) && (seq_no== 0))
		ret=sLspServer->nextConnID;
	else
		ret=conn_id;
	return &ret;
}

LSPMessage* rpc_read_message(Connection *conn, double timeout,unsigned long& addr,unsigned short& port)
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
			rpcInbox_struct inbox;
			if(!sLspServer->rpcInbox.empty())
			{
				sLspServer->rpcInbox.getq(inbox);
				pkt=inbox.lspmsg;
				addr=inbox.addr;
				port=inbox.port;
	//			std::cout<<" host "<<inbox.addr<<"\n";
			}
			if(network_should_drop())
				continue; // drop the packet and continue reading
			return pkt;
		}
	}
}


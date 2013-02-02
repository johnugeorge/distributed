#include<pthread.h>
#include "lsp.h"
#include "epoch_handler.h"
//#include "lspmessage.pb-c.h"

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
  void *c_epoch_timer(void*);
  lsp_client* new_client = new lsp_client;
  pthread_create(&client_epoch, NULL, c_epoch_timer, (void*) new_client);
  //pthread_join(client_epoch, NULL);
}

int lsp_client_read(lsp_client* a_client, uint8_t* pld)
{
	
}

bool lsp_client_write(lsp_client* a_client, uint8_t* pld, int lth)
{

}

bool lsp_client_close(lsp_client* a_client)
{
	
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

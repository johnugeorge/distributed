#include "lsp.h"
#include<string>
using namespace std;


void handle_read(uint8_t* buffer)
{
cout<<" Message Type "<<buffer[0]<<"\n";


}


int main(int argc, char** argv) 
{
	uint8_t buffer[MAX_PAYLOAD_SIZE];	
	if(argc != 2 )
	{
		std::cout<<" Wrong Arguments. Usage ./worker host:port\n";
		exit(0);
	}

	string str(argv[1]);
	string host,port;
	unsigned  found=str.find(':');
	if (found!=std::string::npos)
	{
		host=str.substr(0,found);
		port=str.substr(found+1);
	}
	else
	{
		std::cout<<" Wrong Arguments. Usage ./worker host:port \n";
	}
	cout<<host<<" "<<port<<"\n";
	lsp_client* myclient = lsp_client_create(host.c_str(), atoi(port.c_str()));
	const char join_message[]="j";
	lsp_client_write(myclient, (uint8_t*) join_message, strlen(join_message));
	

	while(1) {

	int numbytes=(lsp_client_read(myclient, buffer));
	if(numbytes > 0)
	{
		std::cout<<" buffer "<<buffer<<" bytes rcvd "<<numbytes<<"\n";
		handle_read(buffer);
		bzero(buffer,MAX_PAYLOAD_SIZE);

	}
	else if(numbytes == -1)
	{

		std::cout<<" Server disconnected.Hence shutting down \n";
//	        exit(0);
	}
	             
	//puts((const char*)buffer);
	}
	lsp_client_close(myclient);
	
	printf("End of client code\n");
	return 0;
}


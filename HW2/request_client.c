#include "lsp.h"
#include<string>
using namespace std;


void handle_read(uint8_t* buffer)
{
	cout<<" Message Type "<<buffer[0]<<"\n";

   if(buffer[0] == 'f')
   {

   }
   else if (buffer[0] == 'x')
   {

   }
   else
   {

   }

}

int main(int argc, char** argv) 
{
	uint8_t buffer[MAX_PAYLOAD_SIZE];	
	if(argc != 4 )
	{
		std::cout<<" Wrong Arguments. Usage ./request host:port hash len\n";
		exit(0);
	}
	string upper="";
	string lower="";
	string str(argv[1]);
	string host,port;
	unsigned  found=str.find(':');
	string hash(argv[2]);
	string len(argv[3]);
	int length=atoi(len.c_str());

	if (found!=std::string::npos)
	{
		host=str.substr(0,found);
		port=str.substr(found+1);
	}
	else
	{
		std::cout<<" Wrong Arguments. Usage ./request host:port hash len\n";
		exit(0);
	}
	cout<<host<<" "<<port<<" "<<hash<<" "<<length<<"\n";
	lsp_client* myclient = lsp_client_create(host.c_str(), atoi(port.c_str()));
	for(int l=0;l<length;l++)
	{
		lower=lower+"a";
	        upper=upper+"z";
	}
	string crack_msg= string("c")+" "+hash+" "+lower+" "+upper;
	cout<<" Crack Message "<<crack_msg<<"\n";
	lsp_client_write(myclient, (uint8_t*) crack_msg.c_str(), strlen(crack_msg.c_str()));
	
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

		std::cout<<" Disconnected \n";
		break;
	}
	             
	//puts((const char*)buffer);
	}
	lsp_client_close(myclient);
	
	printf("End of client code\n");
	return 0;
}



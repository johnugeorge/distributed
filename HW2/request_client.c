#include "lsp.h"
#include<string>
using namespace std;


ofstream outFile("request_client.txt");
bool handle_read(uint8_t* buffer)
{
   string str((const char*)buffer);
   if(buffer[0] == 'f')
   {
      PRINT(LOG_INFO," Found: "<<str.substr(2));
   }
   else if (buffer[0] == 'x')
   {
       PRINT(LOG_INFO," Not Found ");
   }
   else
   {
       PRINT(LOG_INFO," Unknown message Type "<<buffer[0]);
      return false;
   }
   return true;

}

int main(int argc, char** argv) 
{
	/*lsp_set_drop_rate(_DROP_RATE);
	lsp_set_epoch_cnt(_EPOCH_CNT);
	lsp_set_epoch_lth(_EPOCH_LTH);*/
	initialize_configuration();
	srand(12345);
	uint8_t buffer[MAX_PAYLOAD_SIZE];	
	if(argc != 4 )
	{
		PRINT(LOG_INFO, " Wrong Arguments. Usage ./request host:port hash len\n");
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
		PRINT(LOG_INFO, " Wrong Arguments. Usage ./request host:port hash len\n");
		exit(0);
	}
	 PRINT(LOG_INFO,host<<" "<<port<<" "<<hash<<" "<<length);
	lsp_client* myclient = lsp_client_create(host.c_str(), atoi(port.c_str()));
	for(int l=0;l<length;l++)
	{
		lower=lower+"a";
	        upper=upper+"z";
	}
	string crack_msg= string("c")+" "+hash+" "+lower+" "+upper;
	PRINT(LOG_INFO," Crack Message "<<crack_msg);
	lsp_client_write(myclient, (uint8_t*) crack_msg.c_str(), strlen(crack_msg.c_str()));
	int result_not_recvd=false;
	while(1) {

	int numbytes=(lsp_client_read(myclient, buffer));
	if(numbytes > 0)
	{
		PRINT(LOG_INFO, " buffer "<<buffer<<" bytes rcvd "<<numbytes<<"\n");
		result_not_recvd=handle_read(buffer);
                if(result_not_recvd == true)break;
		bzero(buffer,MAX_PAYLOAD_SIZE);
	}
	else if(numbytes == -1)
	{

		PRINT(LOG_INFO, " Disconnected \n");
		break;
	}
	             
	//puts((const char*)buffer);
	}
	lsp_client_close(myclient);
	
	printf("End of client code\n");
	return 0;
}



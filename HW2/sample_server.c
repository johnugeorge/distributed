#include "lsp.h"

int main(int argc, char** argv) 
{
	lsp_server* myserver = lsp_server_create(atoi(argv[1]));
	
	uint8_t payload[MAX_PAYLOAD_SIZE];
	uint32_t returned_id;
	int bytes_read;
	int count=0;	
	for(;;)
	{
		//printf("Issuing read\n");
		bzero(payload,MAX_PAYLOAD_SIZE);
		int bytes = lsp_server_read(myserver, payload, &returned_id);
//		sleep(5);
		//	Echo it right back
		if(returned_id != 0 && bytes > 0)
		{
			std::cout<<" conn Id in Application: "<<returned_id<<" payload: "<<payload<<" bytes revcd: "<<bytes<<" strlen "<<strlen((char*)payload)<<"\n";
			lsp_server_write(myserver, payload, bytes, returned_id);
			count++;
		}
		else if (returned_id !=0 && bytes == -1)
		{
		        std::cout<<" client with conn_id "<<returned_id<<" shuts down "<<payload<<"bytes "<<bytes<<"\n";
		//	exit(0);

		}
//else
//		        std::cout<<" No bytes to be read :conn Id in Application "<<returned_id<<" payload "<<payload<<"bytes "<<bytes<<"\n";
	}
	
	return 0;
}

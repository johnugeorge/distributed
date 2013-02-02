#include "lsp.h"

int main(int argc, char** argv) 
{
	lsp_server* myserver = lsp_server_create(atoi(argv[1]));
	
	uint8_t payload[4096];
	uint32_t returned_id;
	int bytes_read;
	
	for(;;)
	{
		printf("Issuing read\n");
		int bytes = lsp_server_read(myserver, payload, &returned_id);
		sleep(5);
		//	Echo it right back
		lsp_server_write(myserver, payload, bytes, returned_id);
	}
	
	return 0;
}

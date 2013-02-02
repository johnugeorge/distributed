#include "lsp.h"

int main(int argc, char** argv) 
{
	lsp_server* myserver = lsp_server_create(atoi(argv[1]));
	
	char payload[4096];
	int returned_id;
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

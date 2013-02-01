#include "lsp.h"


int main(int argc, char** argv) 
{
	lsp_client* myclient = lsp_client_create("127.0.0.1", atoi(argv[1]));
	
	char message[] = "ilovethiscoursealready";

	lsp_client_write(myclient, (char *) message, strlen(message));
	
	uint8_t buffer[4096];
	int bytes_read = lsp_client_read(myclient, buffer);
	
	puts(buffer);
	
	lsp_client_close(myclient);
	
	printf("End of client code\n");
	return 0;
}

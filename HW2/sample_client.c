#include "lsp.h"


int main(int argc, char** argv) 
{
	lsp_client* myclient = lsp_client_create("127.0.0.1", atoi(argv[1]));

	const char message[] = "ilovethiscoursealready";

	lsp_client_write(myclient, (uint8_t*) message, strlen(message));

	const char message1[] = "This is a test";

	lsp_client_write(myclient, (uint8_t*) message1, strlen(message1));
        const char message2[] = "test passed";

	lsp_client_write(myclient, (uint8_t*) message2, strlen(message2));
	
	uint8_t buffer[MAX_PAYLOAD_SIZE];	
	while(1) {

	int numbytes=(lsp_client_read(myclient, buffer));
	if(numbytes > 0)
	{
		std::cout<<" buffer "<<buffer<<" bytes rcvd "<<numbytes<<"\n";
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

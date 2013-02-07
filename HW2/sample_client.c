#include "lsp.h"


int main(int argc, char** argv) 
{
	lsp_client* myclient = lsp_client_create("127.0.0.1", atoi(argv[1]));

//sleep(1);	
	const char message[] = "ilovethiscoursealready";
	std::cout<<" strlen1  "<<strlen(message)<<"\n";

	lsp_client_write(myclient, (uint8_t*) message, strlen(message));

//sleep(1);
	const char message1[] = "This is a test";
	std::cout<<" strlen1  "<<strlen(message1)<<"\n";

	lsp_client_write(myclient, (uint8_t*) message1, strlen(message1));
//sleep(1);
        const char message2[] = "test passed";
	std::cout<<" strlen1  "<<strlen(message2)<<"\n";

	lsp_client_write(myclient, (uint8_t*) message2, strlen(message2));
	
sleep(10);
	uint8_t buffer[MAX_PAYLOAD_SIZE];
	
	while(1) {

	int numbytes=(lsp_client_read(myclient, buffer));
	if(numbytes!=0)
	{
	std::cout<<" buffer "<<buffer<<" bytes rcvd "<<numbytes<<" strlen "<<strlen((char*)buffer)<<"\n";
	bzero(buffer,MAX_PAYLOAD_SIZE);

	}
	//puts((const char*)buffer);
	}
	lsp_client_close(myclient);
	
	printf("End of client code\n");
	return 0;
}

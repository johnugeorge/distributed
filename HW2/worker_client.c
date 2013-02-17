#include "lsp.h"
#include<string>
#include <openssl/sha.h>

using namespace std;

string findReverseHash(string hash_str,string lower,string upper)
{
	string start=lower;
	int last_char_pos=start.length()-1;
	int start_char_pos=start.length()-1;
	size_t length = lower.length();
	unsigned char hash[20];
	char temp[20];
	int found=0;
	int end=0;

	while(!found && !end)
	{
		if(start.compare(upper) == 0)
			end=1;

		if(start[last_char_pos]<='z')
		{
			//cout<<"string "<<start<<"\n"; 
			SHA1((const unsigned char*)start.c_str(), length, hash);
			int i;
			for (i = 0; i < 20; i++) {
				sprintf(temp,"%02x",hash[i]);
				string temp1=hash_str.substr(i*2,2);
				if(memcmp(temp,temp1.c_str(),2))
				{
					break;
				}
			}
			//	printf("\n");
			if(i==20) 
			{
				found=1;
				continue;
			}
			start[last_char_pos]+=1;

		}
		else
		{
			int j=last_char_pos-1;

			while(j>=0)
			{
				if(start[j]=='z')
				{
					if(j==0)
					{
						end=1;
						break;
					}
					j--;
				}
				else
				{
					start[j]+=1;
					start=start.substr(0,j+1)+lower.substr(j+1);
					last_char_pos=length-1;
					break;
				}

			}
		}

	}
        
        string res;
	if(found==1)
	{
                res.append("f ");
                res.append(start);
		cout<<" Found: "<<start<<"\n";
	}
	else
	{
                res.append("x");
		cout<<" Not Found \n";

	}

        return res;
}
string handle_read(uint8_t* buffer)
{
cout<<" Message Type "<<buffer[0]<<"\n";
vector<string> tokens;
if(buffer[0]=='c')
{
	string str((const char*)buffer);
	string delimiters=" ";
	string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	string::size_type pos     = str.find_first_of(delimiters, lastPos);
	while (string::npos != pos || string::npos != lastPos)
	{
		// Found a token, add it to the vector.
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		//                 // Skip delimiters.  Note the "not_of"
		lastPos = str.find_first_not_of(delimiters, pos);
		// Find next "non-delimiter"
		pos = str.find_first_of(delimiters, lastPos);
	}
}
else
{
	cout<<"Unknown Message type "<<buffer[0]<<" for worker \n";
	return "";
}
	 cout<<tokens[0]<<" "<<tokens[1]<<" "<<tokens[2]<<" "<<tokens[3]<<"\n";
return findReverseHash(tokens[1],tokens[2],tokens[3]);
}


int main(int argc, char** argv) 
{
	/*lsp_set_drop_rate(_DROP_RATE);
	lsp_set_epoch_cnt(_EPOCH_CNT);
	lsp_set_epoch_lth(_EPOCH_LTH);*/
        initialize_configuration();
	srand(12345);
	uint8_t buffer[MAX_PAYLOAD_SIZE];	
	if(argc != 2 )
	{
		PRINT(LOG_INFO, " Wrong Arguments. Usage ./worker host:port\n");
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
		PRINT(LOG_INFO, " Wrong Arguments. Usage ./worker host:port \n");
	}
	cout<<host<<" "<<port<<"\n";
	lsp_client* myclient = lsp_client_create(host.c_str(), atoi(port.c_str()));
	const char join_message[]="j";
	lsp_client_write(myclient, (uint8_t*) join_message, strlen(join_message));
	

	while(1) {

	int numbytes=(lsp_client_read(myclient, buffer));
	if(numbytes > 0)
	{
		PRINT(LOG_INFO, " buffer "<<buffer<<" bytes rcvd "<<numbytes<<"\n");
		string res = handle_read(buffer);
		bzero(buffer,MAX_PAYLOAD_SIZE);
                
                if(res == "") continue;
                lsp_client_write(myclient, (uint8_t*)res.c_str(), strlen(res.c_str()));
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


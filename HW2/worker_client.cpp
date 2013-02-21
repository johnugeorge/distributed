#include "lsp_client.h"
#include "server_utils.h"
#include <string>
#include <openssl/sha.h>

using namespace std;
ofstream outFile("worker_client.log");
pthread_t junior_wkr1;
pthread_t junior_wkr2;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
vector<string> thread_res;

typedef struct request
{
  string c;
  string hash;
  string lower;
  string upper;
} request;

void* findReverseHash(void* r)
{
  request* req = (request*)r;
  string hash_str = req->hash;
  string lower = req->lower;
  string upper = req->upper;
  string start = lower;
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

    if(start[last_char_pos] <='z')
    {
      SHA1((const unsigned char*)start.c_str(), length, hash);
      int i;
      for(i = 0; i < 20; i++) 
      {
        sprintf(temp,"%02x",hash[i]);
        string temp1=hash_str.substr(i*2,2);
        if(memcmp(temp,temp1.c_str(),2))
          break;
      }
      if(i == 20) 
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
	if(start[j] == 'z')
	{
          if(j == 0)
	  {
	    end = 1;
	    break;
	  }
	  j--;
    	}
	else
        {
	  start[j] += 1;
	  start=start.substr(0, j+1)+lower.substr(j+1);
	  last_char_pos = length-1;
	  break;
	}
      }
    }
  }
        
  string res;
  if(found == 1)
  {
    res.append("f ");
    res.append(start);
    PRINT(LOG_INFO,"Found: "<<start);
    pthread_mutex_lock(&lock);  
    thread_res.push_back(res);
    pthread_mutex_unlock(&lock);
  }
  else
  {
    res.append("x");
    PRINT(LOG_INFO,"Not Found ");
    pthread_mutex_lock(&lock);  
    thread_res.push_back(res);
    pthread_mutex_unlock(&lock);
  }

  //return res;
}


vector<string> divide_for_threading(string lower, string upper)
{
  /*
   * aaaa-bzzz gets split into aaaa-azzz, baaa-bzzz
   * caaa-dzzz gets split into caaa-czzz, daaa-dzzz
   */
  string hyp("-");
  vector<string> res;
  //vector<string> bounds = strsplit(task, hyp);
  //string lower = bounds[0];
  //string upper = bounds[1];
  int pwd_len = lower.length();
  int th_l = 2;
  int i = 0;
  char first = lower[0];
  char second = upper[0];
  char a = 'a';
  char z = 'z';
  while(i < th_l)
  {
    string new_task1;
    new_task1.append(1, first);
    new_task1.append(pwd_len-1, a);
    res.push_back(new_task1);
    //new_task.append(hyp);
    string new_task2;
    new_task2.append(1, first);
    new_task2.append(pwd_len-1, z);
    first = second;
    res.push_back(new_task2);
    i++;
  }

  return res;
}


string handle_read(uint8_t* buffer)
{
  PRINT(LOG_INFO,"Message type "<<buffer[0]);
  if(thread_res.size() > 0)
    thread_res.clear();
  
  vector<string> tokens;
  request* req = new request;

  if(buffer[0] == 'c')
  {
    string str((const char*)buffer);
    string delimiters = " ";

    tokens = strsplit(str, delimiters);

    //init the request
    req->c = tokens[0];
    req->hash = tokens[1];
    req->lower = tokens[2];
    req->upper = tokens[3];

    int p_len = tokens[2].length();

    vector<string> tokens_copy(tokens);
    if(p_len > 3)
    {
      //init a copy of the req
      //request req_copy;
      request* req_copy = new request;
      /*req_copy.c = req.c;
      req_copy.hash = req.hash;
      req_copy.lower = req.lower;
      req_copy.upper = req.upper;*/
      req_copy->c = req->c;
      req_copy->hash = req->hash;
      req_copy->lower = req->lower;
      req_copy->upper = req->upper;

      vector<string> v = divide_for_threading(tokens[2], tokens[3]);
      req_copy->lower = v[2];
      req_copy->upper = v[3];
      req->lower = v[0];
      req->upper = v[1];

      void* findReverseHash(void*);
      pthread_create(&junior_wkr1, NULL, findReverseHash, (void*)req);
      pthread_create(&junior_wkr2, NULL, findReverseHash, (void*)req_copy);
      pthread_join(junior_wkr1, NULL);
      pthread_join(junior_wkr2, NULL);

      //=== observer the results
      int n = thread_res.size();
      int i = 0;
      bool found = false;
      while(i < n)
      {
        string x = thread_res[i];
        if(x[0] == 'f')
        {
          return x;
        }
        i++;
      }

      //if we reach here it means all results are 'x', so just return first one
      return thread_res[0];
    }
    /*string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);
    while(string::npos != pos || string::npos != lastPos)
    {
      // Found a token, add it to the vector.
      tokens.push_back(str.substr(lastPos, pos - lastPos));
      
      // Skip delimiters.  Note the "not_of"
      lastPos = str.find_first_not_of(delimiters, pos);
      
      // Find next "non-delimiter"
      pos = str.find_first_of(delimiters, lastPos);
    }*/
  }
  else
  {
    PRINT(LOG_INFO, "Unknown message type "<<buffer[0]<<" for worker");
    return "";
  }
  
  PRINT(LOG_INFO, tokens[0]<<" "<<tokens[1]<<" "<<tokens[2]<<" "<<tokens[3]); 
  findReverseHash(req);
  return thread_res[0];
}


int main(int argc, char** argv) 
{
  /*string t1 = "c 2c1acb19ec50e7cf120b537395724ee2b2ae98fb yaaaa zzzzz";
  const char* x = t1.c_str();
  string res = handle_read((uint8_t*)x);
  cout<<"Result: "<<res<<endl;
  string t2 = "c 8f890318cdb4079b0ac999f477ea8807ffd0fdbd aaa zzz";
  const char* y = t2.c_str();
  res = handle_read((uint8_t*)y);
  cout<<"Result: "<<res<<endl;
  string t3 = "c 5be93480bd8b743454a93dca084849202af43af5 haaaa izzzz";
  const char* z = t3.c_str();
  res = handle_read((uint8_t*)z);
  cout<<"Result: "<<res<<endl;
  lsp_set_drop_rate(_DROP_RATE);
  lsp_set_epoch_cnt(_EPOCH_CNT);
  lsp_set_epoch_lth(_EPOCH_LTH);*/
  initialize_configuration();
  srand(12345);
  uint8_t buffer[MAX_PAYLOAD_SIZE];	
  if(argc != 2)
  {
    PRINT(LOG_INFO, "Wrong arguments. Usage ./worker host:port");
    exit(0);
  }

  string str(argv[1]);
  string host,port;
  unsigned found=str.find(':');
  if(found!=std::string::npos)
  {
    host=str.substr(0,found);
    port=str.substr(found+1);
  }
  else
  {
    PRINT(LOG_INFO, "Wrong arguments. Usage ./worker host:port");
  }

  PRINT(LOG_INFO,host<<" "<<port);
  lsp_client* myclient = lsp_client_create(host.c_str(), atoi(port.c_str()));

  if(myclient == NULL)
  {
    PRINT(LOG_CRIT,"No connection to server");
    return 0;
  }

  const char join_message[]="j";
  lsp_client_write(myclient, (uint8_t*) join_message, strlen(join_message));
	
  while(1) 
  {
    int numbytes = (lsp_client_read(myclient, buffer));
    if(numbytes > 0)
    {
      PRINT(LOG_INFO, "buffer "<<buffer<<" bytes rcvd "<<numbytes);
      string res = handle_read(buffer);
      bzero(buffer,MAX_PAYLOAD_SIZE);
                
      if(res == "") continue;
      lsp_client_write(myclient, (uint8_t*)res.c_str(), strlen(res.c_str()));
    }
    else if(numbytes == -1)
    {
      PRINT(LOG_INFO, "Disconnected");
      break;
    }
  }
  lsp_client_close(myclient);
	
  PRINT(LOG_INFO, "End of this worker");
  return 0;
}


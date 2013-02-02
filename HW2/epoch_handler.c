#include<iostream>
#include<sys/socket.h>
#include<sys/time.h>
#include<error.h>
#include"lsp.h"
#include"epoch_handler.h"

void c_epoch_timer(void* p)
{
  struct timeval tv;
  int rv;
  char errorbuffer[256];
  //tv.tv_sec = 3;
  //tv.tv_sec = lsp_get_epoch_lth();
  //tv.tv_usec = 0;
  //rv = select(n, &readfds, NULL, NULL, &tv);
  while(1)
  {
    tv.tv_sec = 3;
    //tv.tv_sec = lsp_get_epoch_lth();
    tv.tv_usec = 0;
    rv = select(NULL, NULL, NULL, NULL, &tv);
    
    if(rv == -1)
    {
      fprintf(stderr, "Select error: \n");
      char* errorMessage = strerror_r(errno, errorbuffer, 256);
      printf(errorMessage);
      exit(1);
    }
    else if(rv == 0)
    {
      //timeout occurred, handle it
      c_handle_epoch();
    }
  }
}

void c_handle_epoch()
{
  std::cout<<"Client epoch timer"<<std::endl;  
}

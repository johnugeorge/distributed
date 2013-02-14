#include <iostream>
#include <map>
#include <string>
#include <queue>
#include "server.h"
#include "lsp.h"
#include "server_utils.h"

using namespace std;
void decode_and_dispatch(ServerHandler*, lsp_server*, uint8_t*, uint32_t*, int);


void ServerHandlerHandler::ServerHandlerHandler()
{
  divisions = 13;
}


/*
 * method to handle new crack requests
 */
void ServerHandler::handle_crack(lsp_server* svr, int req_id, string payload)
{
  vector<string> spl = strsplit(req, " ");
  string req = spl[1]; //this is the hash
  string lower = spl[2];

  cache_new_req(req_id, req);
  if(free_workers.empty())
  {
    /* all we can do is cache the request and return */
    return;
  }

  int new_req = request_cache.front();
  string h = request_store[new_req]; //can potentially cause a bug
  int i = 0;
  bool success = false;
  
  //=== init some data structures
  sub_tasks_remaining.insert(new_req, init_sub_tasks());  
  requests_in_progress.push_back(new_req);

  while(!free_workers.empty() || i >= sub_tasks.size())
  {
    w = free_workers.front() //this is also a pop
    //lsp_server_write(svr, req, sub_tasks[i], h);
    register_new_task(svr, w, req_id, i);
    i++;
  }
}


/*
 * method to handle new join requests
 */
void handle_join(int worker_id)
{
  for(r = 0; r < requests_in_progress.size(); r++)
  {
    s = sub_tasks_remaining[r];
    if(!s.empty())
    {
      t = s.front();
      //lsp_server_write(worker_id, t) //and also the hash string
      register_new_task(worker_id, r, t)
      return;
    }
  }
   
  if(request_cache.empty())
  {
    //either no req is in progress or no task is left
    free_workers.push_back(worker_id)
  }
  else
  {
    //there is a req in the cache that can begin working now
    req_id = request_cache.front();
    //lsp_server_write(worker_id, 0); //and also the hash string 
    register_new_task(worker_id, req_id, 0);
    return;
  }
}


/*
 * method to handle the results sent back by workers
 */
void handle_result(int worker_id, TaskResult result)
{
  /* if result is PASS, just return the result to the requester and
   * assign a new request if exists to this worker
   *
   * if it is FAIL, then wait for all other msgs to arrive
   */

  req_id = worker_to_request[worker_id];
  if(!in_vector(request_in_progress, req_id))
  {
    //this means the req_id client could be dead; 
    //or the result of PASS was already sent back to client; so just ignore the result
    return;
  }
    
  remove_subtask(req_id, worker_id, result);

  //if this req has been removed from request_in_progress, then it means that either all subtasks returned fail or a pass was just found
  if(!in_vector(requests_in_progress, req_id))
    ;//lsp_server_write(req_id, result);
}


/*
 * method to handle the event of a dead client
 */
void handle_dead_client(int conn_id)
{
  //first need to figure out if this is a requester or a worker
  bool worker = false;
  int req_id = conn_id;
  if(in_map(conn_id, worker_to_request) || in_vector(conn_id, free_workers))
  {
    worker = true;
    req_id = worker_to_request[conn_id];
  }

  if(worker)
  {
    /* this is a worker who has just been discovered to be dead;
    need to re-assign the task to another worker and modify the registry for this worker */
      
    int task_num = worker_task[conn_id];
    if(free_workers.empty())
    {
      vector<WorkerTask> v = request_divided[req_id];
      vector<WorkerTask>::iterator it = v.begin();
      while(it != v.end())
      {
        if((*it).worker_id == conn_id)
          break;
        it++;
      }
      v.erase(it);
      
      //=== below should check if at all there is an entry for req_id
      if(!in_map(sub_tasks_remaining, req_id))
      {
        vector<int> v2;
        v2.push_back(task_num);
        sub_tasks_remaining.insert(pair<int, vector<int> >(req_id, v2));
      }
      else
        sub_tasks_remaining[req_id].push_back(task_num);
    }
    else
    {
      int i = free_workers.front();
      //lsp_server_write(i, task)
      register_new_task(i, req_id, task_num);

      /* make sure conn_id is removed from everywhere */
    }
  }
  else
  {
    /* this is a requester; so remove it from the request_mapping, so that the
    cracking result is ignored when it arrives */
    request_divided.remove(req_id)
    request_in_progress.remove(req_id)
  }
}


void ServerHandler::cache_new_req(int req_id, string hash_pwd)
{
  request_cache.push_back(req_id);
  request_store.insert(pair<req_id, hash_pwd>);
}


void ServerHandler::register_new_task(int worker_id, int req_id, int task)
{
  worker_task.put(worker_id, task)
  worker_to_request.put(worker_id, req_id)

  // the below call is an UPSERT 
  WorkerTask wt(worker_id, task);
  if(!in_map(request_divided, req_id))
  {
    vector<WorkerTask> v;
    v.push_back(wt);
    request_divided.insert(pair<int, vector<WorkerTask> >(req_id, v));
  }
  else
  {
    vector<WorkerTask> v;
    v = request_divided[req_id];
    v.push_back(wt);
    request_divided[req_id].swap(v);
  }

  // strike off this sub_task
  vector<int> v = sub_tasks_remaining[req_id];
  remove_from_vector(v, task);
  sub_tasks_remaining[req_id].swap(v);
}


void decode_and_dispatch(ServerHandler* svr_handler, 
                         lsp_server* myserver, 
                         uint8_t* payload, 
                         uint32_t* returned_id, 
                         int bytes)
{
  if(*returned_id != 0 && bytes > 0)
  {
    std::cout<<" conn Id in Application: "<<returned_id<<" payload: "<<payload<<" bytes revcd: "<<bytes<<" strlen "<<strlen((char*)payload)<<"\n";
    if(payload[0] == 'c')
    {
      svr_handler->handle_crack(myserver, returned_id, payload);
    }
    else if(payload[0] == 'j')
    {
      svr_handler->handle_join(myserver, returned_id, payload);
    }
    else if(payload[0] == 'f' || payload[0] == 'x')
    {
      int result = (payload[0] == 'f' ? 1 : 0);
    }
    //lsp_server_write(myserver, payload, bytes, returned_id);
  }
  else if(*returned_id != 0 && bytes == -1)
  {
    std::cout<<" client with conn_id "<<returned_id<<" shuts down "<<payload<<"bytes "<<bytes<<"\n";
    svr_handler->handle_dead_client(myserver, returned_id);
  }

}


int main(int argc, char** argv) 
{
   lsp_server* myserver = lsp_server_create(atoi(argv[1]));
   ServerHandler* svr_handler = new ServerHandler();

   uint8_t payload[MAX_PAYLOAD_SIZE];
   uint32_t returned_id;
   int bytes_read;
   int count = 0;

   while(1)
   {
     //printf("Issuing read\n");
     bzero(payload, MAX_PAYLOAD_SIZE);
     int bytes = lsp_server_read(myserver, payload, &returned_id);
     
     //Echo it right back
     if(returned_id != 0 && bytes > 0)
     {
       std::cout<<" conn Id in Application: "<<returned_id<<" payload: "<<payload<<" bytes revcd: "<<bytes<<" strlen "<<strlen((char*)payload)<<"\n";
       //lsp_server_write(myserver, payload, bytes, returned_id);
       count++;
     }
     else if (returned_id !=0 && bytes == -1)
     {
       std::cout<<" client with conn_id "<<returned_id<<" shuts down "<<payload<<"bytes "<<bytes<<"\n";
       //      exit(0);
     }
   }
}

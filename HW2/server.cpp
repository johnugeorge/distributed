#include <iostream>
#include <map>
#include <string>
#include <queue>
#include "server.h"
#include "lsp.h"
#include "server_utils.h"

using namespace std;

void Server::Server()
{
}

void Server::handle_crack(int req_id, string req)
{
  cache_new_req(req_id, req);
  if(free_workers.empty())
  {
    /* all we can do is cache the request and return */
    return;
  }

  int new_req = request_cache.front();
  string h = request_store[new_req]; //can potentially cause a bug
  i = 0;
  bool success = false
  while(!free_workers.empty() || i >= sub_tasks.size())
  {
    w = free_workers.front() //this is also a pop
    lsp_server_write(w, sub_tasks[i], h);
    register_new_task(w, req_id, i);
    i++;
  }
    
  requests_in_progress.push_back(req_id);
}


void handle_join(int worker_id)
{
  for(r = 0; r < requests_in_progress.size(); r++)
  {
    s = sub_tasks_remaining[r];
    if(!s.empty())
    {
      t = s.front();
      lsp_server_write(worker_id, t) //and also the hash string
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
    lsp_server_write(worker_id, 0); //and also the hash string 
    register_new_task(worker_id, req_id, 0);
    return;
  }
}


void handle_result(int worker_id, TaskResult result)
{
  /* if result is PASS, just return the result to the requester and
   * assign a new request if exists to this worker
   *
   * if it is FAIL, then wait for all other msgs to arrive
   */

  req_id = worker_to_request[worker_id];
  if(!inVector(request_in_progress, req_id))
  {
    //this means the req_id client could be dead; 
    //or the result of PASS was already sent back to client; so just ignore the result
    return;
  }
    
  remove_subtask(req_id, worker_id, result);

  //if this req has been removed from request_in_progress, then it means that either all subtasks returned fail or a pass was just found
  if(!inVector(requests_in_progress, req_id))
    lsp_server_write(req_id, result);
}


void handle_dead_client(int conn_id)
{
  //first need to figure out if this is a requester or a worker
  bool worker = false;
  int req_id = conn_id;
  if(inMap(conn_id, worker_to_request) || inVector(conn_id, free_workers))
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
      request_divided.get(req_id).remove(task)
      /*below should check if at all there is an entry for req_id */
      request_remaining.get(req_id).add(task)
    }
    else
    {
      i = free_workers.next()
      success = lsp_server_write(i, task)
      if(success)
        register_new_task(i, req_id, task)

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

/*
 * util function to check if an integer is in an int vector
 *
bool inVector(int data, vector<int>& v)
{
  if(v.empty())
    return false;

  int n = v.size();
  for(int i=0; i<n; i++)
  {
    if(v[i] == data)
      return true;
  }

  return false;
}*/




int main(int argc, char** argv) 
{
  lsp_server* myserver = lsp_server_create(atoi(argv[1]));
	
  uint8_t payload[4096];
  uint32_t returned_id;
  int bytes_read;
	
  while(1)
  {
    int bytes = lsp_server_read(myserver, payload, &returned_id);
    if(bytes == 0)
      continue;
    
    //sleep(5);
    //lsp_server_write(myserver, payload, bytes, returned_id);
  }
	
  return 0;
}

#include <iostream>
#include <map>
#include <string>
#include <queue>
#include <string.h>
#include "server.h"

using namespace std;

ofstream outFile("server.txt");

void decode_and_dispatch(ServerHandler*, lsp_server*, uint8_t*, uint32_t, int);

ServerHandler::ServerHandler(int d)
{
  divisions = d;
}


/*
 * method to handle new crack requests
 */
void ServerHandler::handle_crack(lsp_server* svr, int req_id, uint8_t* payload)
{
  PRINT(LOG_DEBUG, "Entering: ServerHandler::handle_crack");
  string crack_req((const char*)payload); 
  vector<string> spl = strsplit(crack_req, " ");
  string req = spl[1]; //this is the hash
  string lower = spl[2];
  init_subtask_store(req_id, lower.length());

  //PRINT(LOG_INFO, "Subtask store: ");
  //PRINT(LOG_INFO, print_map(sub_task_store));
  cache_new_req(req_id, req);
  if(free_workers.empty())
  {
    /* all we can do is cache the request and return */
    PRINT(LOG_INFO, "No workers available. Request cached.");
    return;
  }

  serve_cached_request(svr);
}


/*
 * this method pops off the front of the request queue, and serves it
 */
void ServerHandler::serve_cached_request(lsp_server* svr)
{
  int new_req = request_cache.front();
  //SubTaskStore store = sub_task_store[new_req];
  PRINT(LOG_INFO, "Server got request "<<new_req<<" from cache\n");
  string h = request_store[new_req]; //can potentially cause a bug
  int i = 0;
  bool success = false;
  
  //=== init some data structures
  sub_tasks_remaining.insert(pair<int, vector<int> >(new_req, sub_task_store[new_req]->new_sub_task_list()));  
  requests_in_progress.push_back(new_req);
  PRINT(LOG_INFO, "Current reqs in progress at the server: "<<print_vector(requests_in_progress));
  PRINT(LOG_INFO, "For request: "<<new_req<<" the remaining sub_tasks are: "<<print_vector(sub_tasks_remaining[new_req]));
  PRINT(LOG_INFO, "Current no of free workers: "<<free_workers.size());

  while(!free_workers.empty() && i < sub_tasks_remaining[new_req].size())
  {
    int w = free_workers.front(); //this is also a pop
    //SubTaskStore store = sub_task_store[new_req];
    int task_num = sub_tasks_remaining[new_req].front();
    string pl = create_crack_payload(h, task_num, sub_task_store[new_req]->sub_task_map());
    PRINT(LOG_INFO, "Server sending payload ["<<pl<<"] to worker "<<w<<", bytes: "<<pl.length()+1);
    lsp_server_write(svr, (void*)pl.c_str(), pl.length(), w);
    register_new_task(w, new_req, task_num);
    PRINT(LOG_INFO, "The remaining sub_tasks for req "<<new_req<<" are: "<<print_vector(sub_tasks_remaining[new_req]));
    i++;
  }
 
  //=== after we're done with new_req, remove it from cache
  vector<int>::iterator it = request_cache.begin();
  request_cache.erase(it);
  PRINT(LOG_DEBUG, "Exiting: ServerHandler::handle_crack");
}


/*
 * method to handle new join requests
 */
void ServerHandler::handle_join(lsp_server* svr,int worker_id)
{
  PRINT(LOG_DEBUG, "Entering: ServerHandler::handle_join");
  for(int r = 0; r < requests_in_progress.size(); r++)
  {
    vector<int> s = sub_tasks_remaining[r];
    if(!s.empty())
    {
      int t = s.front();
      //lsp_server_write(worker_id, t) //and also the hash string
      register_new_task(worker_id, r, t);
      return;
    }
  }
   
  if(request_cache.empty())
  {
    //either no req is in progress or no task is left
    free_workers.push_back(worker_id);
  }
  else
  {
    //there is a req in the cache that can begin working now
    int req_id = request_cache.front();
    //lsp_server_write(worker_id, 0); //and also the hash string 
    register_new_task(worker_id, req_id, 0);
    return;
  }

  PRINT(LOG_DEBUG, "Exiting: ServerHandler::handle_join");
}


/*
 * method to handle the results sent back by workers
 */
void ServerHandler::handle_result(lsp_server* svr, string pwd, int worker_id, TaskResult result)
{
  PRINT(LOG_DEBUG, "Entering: ServerHandler::handle_result");
  PRINT(LOG_INFO, "The worker "<<worker_id<<" has returned");

  /* if result is PASS, just return the result to the requester and
   * assign a new request if exists to this worker
   *
   * if it is FAIL, then wait for all other msgs to arrive
   */

  int req_id = worker_to_request[worker_id];
  if(!in_vector(requests_in_progress, req_id))
  {
    //this means the req_id client could be dead; 
    //or the result of PASS was already sent back to client; so just ignore the result
    PRINT(LOG_INFO, "Ignoring result");
    return;
  }
    
  remove_subtask(req_id, worker_id, result);

  //if this req has been removed from request_in_progress, then it means that either all subtasks returned fail or a pass was just found
  if(!in_vector(requests_in_progress, req_id))
  {
    if(result == PASS)
    {
      vector<string> v;
      v.push_back("f");
      v.push_back(pwd);
      string pl = create_payload(v);
      PRINT(LOG_INFO, "Sending to requester: "<<pl);
      lsp_server_write(svr, (void*)pl.c_str(), pl.length(), req_id);
    }
    else if(result == FAIL)
    {
      vector<string> v;
      v.push_back("x");
      string pl = create_payload(v);
      PRINT(LOG_INFO, "Sending to requester: "<<pl);
      lsp_server_write(svr, (void*)pl.c_str(), pl.length(), req_id);
    }
  }

  //now this worker is free; it can be given a new task
  PRINT(LOG_INFO, "Worker "<<worker_id<<" is free; can be given a new task");
  PRINT(LOG_INFO, "Current requests in progress: "<<print_vector(requests_in_progress));
  for(int i=0; i<requests_in_progress.size(); i++)
  {
    int r = requests_in_progress[i]; 
    PRINT(LOG_INFO, "Request: "<<r);
    vector<int> s = sub_tasks_remaining[r];
    PRINT(LOG_INFO, "Sub tasks remaining: "<<print_vector(s));
    if(!s.empty())
    {
      int t = s.front();
      //SubTaskStore store = sub_task_store[r];
      string pl = create_crack_payload(request_store[r], t, sub_task_store[r]->sub_task_map());
      PRINT(LOG_INFO, "Server sending payload ["<<pl<<"] to worker "<<worker_id<<", bytes: "<<pl.length()+1);
      lsp_server_write(svr, (void*)pl.c_str(), pl.length(), worker_id);
      register_new_task(worker_id, r, t);
      PRINT(LOG_INFO, "For request: "<<r<<" the remaining sub_tasks are: "<<print_vector(sub_tasks_remaining[r]));
      return;
    }
  }
   

  if(request_cache.empty())
  {
    //either no req is in progress or no task is left
    free_workers.push_back(worker_id);
    PRINT(LOG_INFO, "Free workers: "<<print_vector(free_workers));
  }
  else
  {
    //there is a req in the cache that can begin working now
    free_workers.push_back(worker_id);
    serve_cached_request(svr);
  }
}


/*
 * method to handle the event of a dead client
 */
void ServerHandler::handle_dead_client(lsp_server* svr,int conn_id)
{
  //first need to figure out if this is a requester or a worker
  PRINT(LOG_INFO, "Entering ServerHandler::handle_dead_client");
  bool worker = false;
  int req_id = conn_id;
  if(in_map(worker_to_request,conn_id) || in_vector(free_workers,conn_id))
  {
    worker = true;
    req_id = worker_to_request[conn_id];
  }
  
  if(worker)
  {
    /* this is a worker who has just been discovered to be dead; need to re-assign the task to another worker and modify the registry for this worker*/
    int task_num = worker_task[conn_id];
    if(free_workers.empty())
    {
      vector<WorkerTask> v = request_divided[req_id];
      vector<WorkerTask>::iterator it = v.begin();
      while(it != v.end())
      {
        if((*it).conn_id == conn_id)
	{
	  v.erase(it);
	  break;
	}
	it++;
      }
      
      if(it == v.end())
        PRINT(LOG_INFO, "Conn_id "<<conn_id<<" not found");
      
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
    request_divided.erase(req_id);
    remove_from_vector(requests_in_progress,req_id);
    //requests_in_progress.erase(req_id);
  }

  PRINT(LOG_INFO, "Exiting ServerHandler::handle_dead_client");
}


void ServerHandler::cache_new_req(int req_id, string hash_pwd)
{
  request_cache.push_back(req_id);
  request_store.insert(pair<int,string>(req_id, hash_pwd));
}


void ServerHandler::register_new_task(int worker_id, int req_id, int task)
{
  PRINT(LOG_INFO,"Registering new task: worker_id:"<<worker_id<<" req_id:"<<req_id<<" task:"<<task);
  worker_task.insert(pair<int, int>(worker_id, task));
  worker_to_request.insert(pair<int, int>(worker_id, req_id));
  remove_from_vector(free_workers, worker_id);
  
  //=== the below call is an UPSERT 
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

  //=== strike off this sub_task
  vector<int> v = sub_tasks_remaining[req_id];
  remove_from_vector(v, task);
  sub_tasks_remaining[req_id].swap(v);
}


void decode_and_dispatch(ServerHandler* svr_handler, 
                         lsp_server* myserver, 
                         uint8_t* payload, 
                         uint32_t returned_id, 
                         int bytes)
{
  if(returned_id != 0 && bytes > 0)
  {
    PRINT(LOG_INFO, " conn Id in Application: "<<returned_id<<" payload: "<<payload<<" bytes revcd: "<<bytes<<" strlen "<<strlen((char*)payload)<<"\n");
    if(payload[0] == 'c')
    {
      svr_handler->handle_crack(myserver, returned_id, payload);
    }
    else if(payload[0] == 'j')
    {
      svr_handler->handle_join(myserver, returned_id);
    }
    else if(payload[0] == 'f')
    {
      string pl((const char*) payload);
      string pwd = pl.substr(2);
      TaskResult result = PASS;
      svr_handler->handle_result(myserver, pwd, returned_id, result);
    }
    else if(payload[0] == 'x')
    {
      TaskResult result = FAIL;
      svr_handler->handle_result(myserver, "", returned_id, result);
    }
    //lsp_server_write(myserver, payload, bytes, returned_id);
  }
  else if(returned_id != 0 && bytes == -1)
  {
    PRINT(LOG_INFO, " client with conn_id "<<returned_id<<" shuts down "<<payload<<"bytes "<<bytes<<"\n");
    svr_handler->handle_dead_client(myserver, returned_id);
    lsp_server_close(myserver,returned_id);
  }

}


int main(int argc, char** argv) 
{
  /*lsp_set_drop_rate(_DROP_RATE);
  lsp_set_epoch_cnt(_EPOCH_CNT);
  lsp_set_epoch_lth(_EPOCH_LTH);*/
  initialize_configuration();
  srand(12345);

  lsp_server* myserver = lsp_server_create(atoi(argv[1]));
  ServerHandler* svr_handler = new ServerHandler(13);

  uint8_t payload[MAX_PAYLOAD_SIZE];
  uint32_t returned_id;
  int bytes_read;
  int count = 0;

  while(1)
  {
    //printf("Issuing read\n");
    bzero(payload, MAX_PAYLOAD_SIZE);
    int bytes = lsp_server_read(myserver, payload, &returned_id);
    decode_and_dispatch(svr_handler,myserver, payload,returned_id,bytes);
  }
}

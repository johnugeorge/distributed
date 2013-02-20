#include <map>
#include <string>
#include <vector>
#include "lsp.h"
#include "server_utils.h"

using namespace std;

typedef enum TaskResult
{
  FAIL,
  PASS,
  NO_RESULT
} TaskResult;

class WorkerTask
{
  /* everything public to ensure ease of use */
  public:
    int conn_id;
    int task_num;
    TaskResult result;

    WorkerTask(int cid, int tn,TaskResult res)
    {
      conn_id = cid;
      task_num = tn;
      result = res;
    }

    WorkerTask(int cid,int tn)
    {
      conn_id = cid;
      task_num = tn;
      result = NO_RESULT;
    }
};


class SubTaskStore
{
  /*
   * All subtasks start from 1
   */
  public:
    SubTaskStore(int req, int pwd_len) : req_id(req), pwd_length(pwd_len)
    { init(); }

    vector<int> new_sub_task_list()
    {
      return sub_task_names;
    }

    int size()
    {
      //=== once sub_task_names is created, it is not changed.
      //=== to check if a request has finished all its subtasks, the caller needs to check 
      //=== if the no of sub tasks completed are equal to the value returned here
      return (int)sub_task_names.size();
    }

    string sub_task(int task_name)
    {
      return sub_tasks[task_name];
    }

    map<int, string> sub_task_map()
    {
      return sub_tasks;
    }

  private:
    map<int, string> sub_tasks;
    vector<int> sub_task_names;
    int req_id;
    int pwd_length;

    void init()
    {
      vector<string> v = get_divisions(&pwd_length, NULL);
      size_t n = v.size();
      int i = 0;
      while(i < n)
      {
        sub_tasks.insert(pair<int, string>(i+1, v[i]));
        sub_task_names.push_back(i+1);
        i++;
      }
    }
};


class ServerHandler
{
  private:
    map<int, int> worker_task;
    map<int, int> worker_to_request;
    map<int, vector<WorkerTask> > request_divided;
    map<int, vector<int> > sub_tasks_remaining;
    map<int, vector<int> > sub_tasks_completed;
    vector<int> requests_in_progress;
    vector<int> free_workers;
    vector<int> request_cache;
    map<int, string> request_store;
    
    int divisions;
    map<int, SubTaskStore*> sub_task_store;


    /*
     * initializes the subtask store for a particular request, based on the pwd length
     */
    void init_subtask_store(int req_id, int pwd_len)
    {
      PRINT(LOG_INFO, "Entering ServerHandler::init_subtask_store");
      //if(!sub_task_store.empty())
      //  return;

      //vector<string> divs = get_divisions(&pwd_len, &divisions);
      SubTaskStore* store = new SubTaskStore(req_id, pwd_len);
      sub_task_store.insert(pair<int, SubTaskStore*>(req_id, store));

      PRINT(LOG_INFO, "Exiting ServerHandler::init_subtask_store");
    }

    void remove_subtask(int req_id, int worker_id, TaskResult result)
    {
      PRINT(LOG_INFO, "Entering ServerHandler::remove_subtask");
      //PRINT(LOG_INFO, "Requests are currently divided as below.");
      print_current_request_divisions();

      //=== remove the assigned subtask
      map<int, vector<WorkerTask> >::iterator it1 = request_divided.find(req_id);
      vector<WorkerTask> v1 = it1->second;
      vector<WorkerTask>::iterator vit1 = v1.begin();
      int task_num=0;
      while(vit1 != v1.end())
      {
	if((*vit1).conn_id == worker_id)
	{
	  task_num = (*vit1).task_num;
	  v1.erase(vit1);
          request_divided[req_id].swap(v1);
	  break;
	}
	vit1++;
      }

      if(vit1 == v1.end())
        PRINT(LOG_INFO, "Worker Id "<<worker_id<<" not found");
     
      int task = worker_task[worker_id];
      vector<int> v = sub_tasks_completed[req_id];
      v.push_back(task);
      sub_tasks_completed[req_id].swap(v);

      print_current_request_divisions();

      //=== remove the entry from sub tasks remaining
      map<int, vector<int> >::iterator it2 = sub_tasks_remaining.find(req_id);
      map<int, vector<int> >::iterator it3 = sub_tasks_completed.find(req_id);
      vector<int> v2 = it2->second;
      /*vector<int>::iterator vit2 = v2.begin();
      while(vit2 != v2.end())
      {
        if((*vit2) == task_num)
        {
          v2.erase(vit2);
          break;
        }
        vit2++;
      }
  
      if(vit2 == v2.end())
        cout<<"task num Not found \n";
      */
      bool last_subtask = false;
      if(is_request_complete(req_id))
        last_subtask = true;

      //=== other removals
      worker_task.erase(worker_id);
      worker_to_request.erase(worker_id);

      //=== do more things if this was the last remaining subtask or a success
      if(last_subtask || result == PASS)
      {
        vector<int>::iterator vit3 = requests_in_progress.begin();
	while(vit3 != requests_in_progress.end())
	{
          if((*vit3) == req_id)
	  {
            requests_in_progress.erase(vit3);
            PRINT(LOG_INFO, "Request "<<(*vit3)<<" is not in progress anymore");
            break;
	  }
          vit3++;
	}

        //completely wipe out the entry of this request in requests assigned 
        //and sub tasks remaining
        request_divided.erase(it1);
        sub_tasks_remaining.erase(it2);
        sub_tasks_completed.erase(it3);
      }

      PRINT(LOG_INFO, "Exiting ServerHandler::remove_subtask");
    }

    void restore_subtask(int req_id, int prev_worker_id)
    {
      /*
       * this method restores a subtask that was previously assigned to a worker; due to the death of the worker
       */

      PRINT(LOG_INFO, "Entering ServerHandler::restore_subtask");
      int task_num = worker_task[prev_worker_id];
      if(free_workers.empty())
      {
        //=== remove the assigned subtask
        map<int, vector<WorkerTask> >::iterator it1 = request_divided.find(req_id);
        vector<WorkerTask> v1 = it1->second;
        vector<WorkerTask>::iterator vit1 = v1.begin();
	while(vit1 != v1.end())
	{
          if((*vit1).conn_id == prev_worker_id)
	  {
	    v1.erase(vit1);
	    break;
	  }
          vit1++;
	}
        
        if(vit1 == v1.end())
          PRINT(LOG_INFO, "Prev worker id"<<prev_worker_id<<" not found");
        
        //=== now put it back as a pending task
        map<int, vector<int> >::iterator it2 = sub_tasks_remaining.find(req_id);
        vector<int> v2 = it2->second;
        v2.push_back(task_num);
      }

      PRINT(LOG_INFO, "Exiting ServerHandler::restore_subtask");
    }

    void init_divisions(int pwd_length)
    {
      //calculate the divisions
    }

    void print_current_request_divisions()
    {
      PRINT(LOG_DEBUG, "Entering ServerHandler::print_current_request_divisions");

      PRINT(LOG_INFO, "Requests are currently divided as below.");
      map<int, vector<WorkerTask> >& m = request_divided;
      map<int, vector<WorkerTask> >::iterator it = m.begin();
      while(it != m.end())
      {
        vector<WorkerTask> v = it->second;
        size_t n = v.size();
        int i = 0;
        PRINT(LOG_INFO, it->first<<" = ");
        while(i < n)
        {
          WorkerTask w = v[i];
          cout<<"<"<<w.conn_id<<", "<<w.task_num<<"> :: ";
          i++;
        }
        cout<<endl;
        it++;
      }

      PRINT(LOG_INFO, "Exiting print_current_request_divisions");
    }

    /* method to check if all the sub tasks of a particular req have 
     * returned 
     */
    bool is_request_complete(int req_id)
    {
      PRINT(LOG_DEBUG, "Entering method is_request_complete");
      vector<int> v = sub_tasks_completed[req_id];
      bool b = false;
      if(v.size() == divisions)
        b = true;

      PRINT(LOG_DEBUG, "Result: "<<b<<" for request: "<<req_id);
      PRINT(LOG_DEBUG, "Exiting method is_request_complete");
      return b;
    }

  public:
    ServerHandler(int);
    void handle_crack(lsp_server*, int req_id, uint8_t* req);
    void handle_join(lsp_server*, int worker_id);
    void handle_result(lsp_server* svr, string pwd, int worker_id, TaskResult result);
    void handle_dead_client(lsp_server*, int conn_id);
    void register_new_task(int worker_id, int req_id, int task);
    void serve_cached_request(lsp_server*);
    void cache_new_req(int req_id, string request);
    void handle();
};






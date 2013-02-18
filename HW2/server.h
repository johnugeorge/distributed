#include <map>
#include <string>
#include <vector>
#include "server_utils.h"
#include "lsp.h"

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

class ServerHandler
{
  private:
    map<int, int> worker_task;
    map<int, int> worker_to_request;
    map<int, vector<WorkerTask> > request_divided;
    map<int, vector<int> > sub_tasks_remaining;
    vector<int> requests_in_progress;
    vector<int> free_workers;
    vector<int> request_cache;
    map<int, string> request_store;
    
    int divisions;
    map<int, string> sub_task_store;

    void init_subtask_store(int pwd_len)
    {
      if(!sub_task_store.empty())
        return;

      vector<string> divs = get_divisions(&pwd_len, &divisions);
      size_t n = divs.size();
      int i = 0;
      while(i < n)
      {
        sub_task_store.insert(pair<int, string>(i+1, divs[i]));
        i++;
      }
    }

    void remove_subtask(int req_id, int worker_id, TaskResult result)
    {
      PRINT(LOG_INFO, "the worker "<<worker_id<<" has returned"); 
      PRINT(LOG_INFO, "req divided map before");
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
        cout<<"worker Id Not found"<<endl;
      
      cout<<endl;
      print_current_request_divisions();
      cout<<endl;

      //=== remove the entry from sub tasks remaining
      map<int, vector<int> >::iterator it2 = sub_tasks_remaining.find(req_id);
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
      if(v2.empty())
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
	    cout<<"Request "<<(*vit3)<<" is not in progress anymore"<<endl;
            break;
	  }
          vit3++;
	}

        //completely wipe out the entry of this request in requests assigned 
        //and sub tasks remaining
        request_divided.erase(it1);
        sub_tasks_remaining.erase(it2);
      }

      cout<<"Exiting func"<<endl;
    }

    void restore_subtask(int req_id, int prev_worker_id)
    {
      /*
       * this method restores a subtask that was previously assigned to a worker; due to the death of the worker
       */

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
          cout<<" prev worker Id Not found \n";
        
        //=== now put it back as a pending task
        map<int, vector<int> >::iterator it2 = sub_tasks_remaining.find(req_id);
        vector<int> v2 = it2->second;
        v2.push_back(task_num);
      }
    }

    void init_divisions(int pwd_length)
    {
      //calculate the divisions
    }

    void print_current_request_divisions()
    {
      map<int, vector<WorkerTask> >& m = request_divided;
      map<int, vector<WorkerTask> >::iterator it = m.begin();
      while(it != m.end())
      {
        vector<WorkerTask> v = it->second;
        size_t n = v.size();
        int i = 0;
        cout<<it->first<<" = ";
        while(i < n)
        {
          WorkerTask w = v[i];
          cout<<"<"<<w.conn_id<<", "<<w.task_num<<"> :: ";
          i++;
        }
        it++;
      }

      cout<<"Exiting print_current_request_divisions"<<endl;
    }

  public:
    ServerHandler();
    void handle_crack(lsp_server*, int req_id, uint8_t* req);
    void handle_join(lsp_server*, int worker_id);
    void handle_result(lsp_server* svr, string pwd, int worker_id, TaskResult result);
    void handle_dead_client(lsp_server*, int conn_id);
    void register_new_task(int worker_id, int req_id, int task);
    //bool is_task_available();
    void cache_new_req(int req_id, string request);
    void handle();
};






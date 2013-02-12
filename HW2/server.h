#include <map>
#include <string>
#include <vector>
#include "server_utils.h"

#define DIVISIONS 5

using namespace std;

class Server
{
  private:
    map<int, int> worker_task;
    map<int, int> worker_to_request;
    map<int, vector<WorkerTask>& > request_divided;
    map<int, vector<int>& > sub_tasks_remaining;
    vector<int> requests_in_progress;
    vector<int> free_workers;
    vector<int> request_cache;
    map<int, string> request_store;
    string[] sub_tasks;

    void remove_subtask(int req_id, int worker_id, TaskResult result)
    {
      //=== remove the assigned subtask
      map<int, vector<WorkerTask>& >::iterator it1 = request_divided.find(req_id);
      vector<WorkerTask> v1 = it1->second;
      vector<WorkerTask>::iterator vit1 = v1.begin();
      while(vit1 != v1.end())
      {
        if((*vit1).worker_id == worker_id)
          break;
        vit1++;
      }
      int task_num = (*vit1).task_num;
      v1.erase(vit1);

      //=== remove the entry from sub tasks remaining
      map<int, vector<int>& >::iterator it2 = sub_tasks_remaining.find(req_id);
      vector<int> v2 = it2->second;
      vector<int>::iterator vit2 = v2.begin();
      while(vit2 != v2.end())
      {
        if((*vit2) == task_num)
          break;
        vit2++;
      }
      v2.erase(vit2);
      bool last_subtask = false;
      if(v2.empty())
        last_subtask = true;

      //=== other removals
      worker_task.erase(worker_id);
      worker_to_request.erase(worker_id);

      //=== do more things if this was the last remaining subtask or a success
      if(last_subtask || TaskResult == PASS)
      {
        vector<int>::iterator vit3 = requests_in_progress.begin();
        while(vit3 != requests_in_progress.end())
        {
          if((*vit3) == req_id)
            break;
          vit3++;
        }

        if(vit != requests_in_progress.end())
          requests_in_progress.erase(vit3);

        //completely wipe out the entry of this request in requests assigned 
        //and sub tasks remaining
        requests_divided.erase(it1);
        sub_tasks_remaining.erase(it2);
      }
    }

    void restore_subtask(int req_id, int prev_worker_id, int task_num)
    {
      /*
       * this method restores a subtask that was previously assigned to a worker; due to the death of the worker
       */

      int task_num = worker_task[req_id];
      if(free_workers.empty())
      {
        //=== remove the assigned subtask
        map<int, vector<WorkerTask>& >::iterator it1 = request_divided.find(req_id);
        vector<WorkerTask> v1 = it1->second;
        vector<WorkerTask>::iterator vit1 = v1.begin();
        while(vit1 != v1.end())
        {
          if((*vit1).worker_id == worker_id)
            break;
          vit1++;
        }
        v1.erase(vit1);
        
        //=== now put it back as a pending task
        map<int, vector<int>& >::iterator it2 = sub_tasks_remaining.find(req_id);
        vector<int> v2 = it2->second;
        v2.push_back(task_num);
      }
    }

  public:
    Server();
    void handle_crack(int req_id, string req);
    void handle_join(int worker_id);
    void handle_result(int worker_id, int result);
    void handle_dead_client(int conn_id);
    void register_new_task(int worker_id, int req_id, int task);
    bool is_task_available();
    void cache_new_req(int req_id, string request);
};

class WorkerTask
{
  /* everything public to ensure ease of use */
  public:
    int conn_id;
    int task_num;
    TaskResult result;
};

enum TaskResult
{
  FAIL,
  PASS,
  NO_RESULT
};
}

#include "simple_scheduler.h"
using namespace std;

/***********/
//FCFS
/***********/
void FCFSScheduler::add_process(int pros) {
  run_queue_.push_back(pros);
}

int FCFSScheduler::get_next_process() {
  if (run_queue_.size() > 0){
    int pros = run_queue_.front();
    run_queue_.pop_front();
    return pros;
  } else{
    return -1;
  }
  
}

void FCFSScheduler::PrintAllEvent(){
  cout << "sched_size: " << run_queue_.size() << endl;
}

bool FCFSScheduler::test_preempt(int pros){
  return false;
}

/***********/
//LCFS
/***********/
void LCFSScheduler::add_process(int pros) {
  run_queue_.push_back(pros);
}

int LCFSScheduler::get_next_process() {
  if (run_queue_.size() > 0){
    int pros = run_queue_.back();
    run_queue_.pop_back();
    return pros;
  } else{
    return -1;
  }
}

void LCFSScheduler::PrintAllEvent(){
  cout << "sched_size: " << run_queue_.size() << endl;
}

bool LCFSScheduler::test_preempt(int pros){
  return false;
}
/***********/
//SRTF
/***********/
void SRTFScheduler::add_process(int pros) {
  for (list<int>::iterator it = run_queue_.begin();
        it != run_queue_.end(); it++){
      // cout << *it << endl;
    /* */
    if (process_list_[*it].time_left_cpu > process_list_[pros].time_left_cpu){
      run_queue_.insert(it, pros);
      return;
    } 
    
  }
  run_queue_.push_back(pros);
}

int SRTFScheduler::get_next_process() {
  if (run_queue_.size() > 0){
    int pros = run_queue_.front();
    run_queue_.pop_front();
    return pros;
  } else{
    return -1;
  }
  
}

void SRTFScheduler::PrintAllEvent(){
  cout << "sched_size: " << run_queue_.size() << endl;
}

bool SRTFScheduler::test_preempt(int pros){
  return false;
}

/*******************/
//RR equal to FCFS
/*******************/


/*************/
//PRIO 
/*************/

void PRIOScheduler::add_process(int pros) {
  if (process_list_[pros].dynamic_prio == -1){
    int dynamic_prio = process_list_[pros].prio - 1;
    process_list_[pros].dynamic_prio = dynamic_prio;
    (*expire_queue_)[dynamic_prio].push_back(pros);
  } else{
    int dynamic_prio = process_list_[pros].dynamic_prio;
    (*active_queue_)[dynamic_prio].push_back(pros);
  }
}

int PRIOScheduler::get_next_process() {
  //determine if switch queues is needed
  bool is_active_queue_empty = true;
  for (int i = 0; i < (*active_queue_).size(); i++){
    if(!(*active_queue_)[i].empty()){
      is_active_queue_empty = false;
      break;
    }
  }
  if (is_active_queue_empty){
    vector<list<int>>* temp_queue = active_queue_;
    active_queue_ = expire_queue_;
    expire_queue_ = temp_queue;
  }
  // first i is active_queue_.size() - 1
  for (int i = (*active_queue_).size(); i-- > 0;){
    if ((*active_queue_)[i].empty()){
      continue;
    }
    int pros = (*active_queue_)[i].front();
    (*active_queue_)[i].pop_front();
    return pros;
  }
  return -1;
}

void PRIOScheduler::PrintAllEvent(){
  cout << "sched_size: " << (*active_queue_).size() << endl;
}

bool PRIOScheduler::test_preempt(int pros){
  return false;
}
/****************/
//PREPRIO
/****************/
bool PREPRIOScheduler::test_preempt(int pros){
  return true;
}
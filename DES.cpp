#include <iostream>
#include <vector>
#include <list>

#include "DES.h"
#include "process.h"

using namespace std;

DiscreteEventSimulator::DiscreteEventSimulator(vector<Process> processes) {
  processes_ = processes;

  // create the event queue
  for (int i = 0; i < processes_.size(); i++){
    Event event;
    event.pid = processes_[i].pid;
    event.new_state = READY;
    event.old_state = CREATED;
    event.time_stamp = processes_[i].arrival_time;
    DiscreteEventSimulator::AddEvent(event);
  }
}

void DiscreteEventSimulator::PrintAllProcess() {
  /* print process vector */
  for (int i = 0; i < processes_.size(); i++) {
    cout << "Process:";
    cout << processes_[i].pid << ":";
    cout << processes_[i].arrival_time << ":";
    cout << processes_[i].total_cpu_time << ":";
    cout << processes_[i].cpu_burst << ":";
    cout << processes_[i].io_burst << endl;
  }
}

void DiscreteEventSimulator::AddEvent(Event event){
  for (list<Event>::iterator it = event_queue_.begin();
       it != event_queue_.end(); it++ ){
    if (it->time_stamp > event.time_stamp){
      event_queue_.insert(it, event);
      return;
    }
  }
  event_queue_.push_back(event);
}


Event* DiscreteEventSimulator::GetEvent(){  
  //find the smallest time stamp item
  if (event_queue_.empty()){
    return NULL;
  }
  return &(event_queue_.front());
}

void DiscreteEventSimulator::RemoveEvent(Event* event){  
  event_queue_.remove(*event);
}



//preempt
int DiscreteEventSimulator::CancelEvent(int pros){
  for (list<Event>::iterator it = event_queue_.begin();
       it != event_queue_.end(); it++){
    if (it->pid == pros){
      int removed_event_timestamp = it->time_stamp;
      event_queue_.remove(*it);
      return removed_event_timestamp;
    }
  }
  return 0;
}

void DiscreteEventSimulator::PrintAllEvent(){
  cout << "event_queue_size: " << event_queue_.size() << endl;
}
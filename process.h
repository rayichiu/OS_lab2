#ifndef PROCESS_H
#define PROCESS_H

struct Process{
  int pid;
  int arrival_time;
  int total_cpu_time;
  int cpu_burst;
  int io_burst;
  // adding process condition for output
  int finish_time;
  int IO_total;
  int prio;
  int cpu_wait_time;
  // adding process condition for executing
  int time_left_cpu;
  int time_left_cpu_burst;
  int dynamic_prio;
  int time_in_curr_state;
  int curr_state_timestamp;
};

enum ProcessState { CREATED, READY, RUNNING, BLOCK, DONE, PREEMPTED };

struct Event {
  int time_stamp;
  int pid;
  ProcessState old_state;
  ProcessState new_state;

  bool operator==(const Event& event) const {
    return this->time_stamp == event.time_stamp &&
           this->pid == event.pid &&
           this->old_state == event.old_state &&
           this->new_state == event.new_state;
  }
};

#endif  //PROCESS_H
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <cstring>
#include <stdlib.h>
#include <unistd.h>

#include "process.h"
#include "DES.h"
#include "scheduler.h"
#include "simple_scheduler.h"

using namespace std;

/***********************************/
// Global Variables
/***********************************/
// flag name
bool vflag = false;
bool tflag = false;
bool eflag = false;
bool pflag = false;
char scheduler_type;
string scheduler_name;
int t_quantum = 10000;
int maxprio = 4;
bool is_prio = false;

/***********************************/
// Read File
/***********************************/
vector<Process> ReadFile(const string& filename) {
  fstream file;
  file.open(filename, ios::in);
  int pid_count = 0;
  vector<Process> process_list;
  string tmp_line;
  while (getline(file, tmp_line)){
    istringstream iss(tmp_line);
    int arrival_time, total_cpu_time, cpu_burst, io_burst;
    if ((iss >> arrival_time >> total_cpu_time >>
          cpu_burst >> io_burst)){
      Process process;
      process.pid = pid_count;
      process.arrival_time = arrival_time;
      process.total_cpu_time = total_cpu_time;
      process.cpu_burst = cpu_burst;
      process.io_burst = io_burst;

      process.finish_time = 0;
      process.IO_total = 0;
      process.prio = 4;
      process.cpu_wait_time = 0;

      process.time_left_cpu = total_cpu_time;
      process.time_left_cpu_burst = 0;
      process.dynamic_prio = 4;
      process.time_in_curr_state = 0;
      process.curr_state_timestamp = arrival_time;

      process_list.push_back(process);
      pid_count++;      
    } else{
      printf("input error");
      exit(EXIT_FAILURE);
    }
  }
  file.close();
  return process_list;
}

/***********************************/
// Read Rfile
/***********************************/
class RandomNumberGenerator{
  public:
    RandomNumberGenerator(string rfilename){
      fstream rfile;
      rfile.open(rfilename, ios::in);
      bool random_file_len = true;
      while(!rfile.eof()){
        int x;
        rfile >> x;
        if (rfile.eof()) break;
        if (random_file_len){
          random_file_len = false;
        } else{
          random_number_list_.push_back(x);
        } 
      }
      rfile.close();
    }

    int GetNext(int burst) {
      int ofs_curr = ofs_;
      ofs_++;
      if (ofs_ >= random_number_list_.size()){
        ofs_ = ofs_ - random_number_list_.size();
      }
      return int(1 + (random_number_list_[ofs_curr] % burst));
    }

  private:
    int ofs_ = 0;
    vector<int> random_number_list_;
};

/***********************************/
// Operator Overload compare enum
/***********************************/
std::ostream& operator<<(std::ostream& out, const ProcessState state){
    switch (state) {
      case CREATED:
        return out << "CREATED";
      case READY:
        return out << "READY";
      case RUNNING:
        return out << "RUNNG";
      case BLOCK:
        return out << "BLOCK";
      case DONE:
        return out << "Done";
      case PREEMPTED:
        return out << "PREEMPTED";
    }
}

/**************************************/
// process cmd
/**************************************/
void process_cmd(int argc, char* argv[]){
  int o;
  // const char* optstring = "vteps:";
  while ((o = getopt(argc, argv, "vteps:")) != -1){
    switch (o) {
      case 'v':
        //verbose
        vflag = true;
        break;
      case 't':
        //event execution
        tflag = true;
        break;
      case 'e':
        //event queue
        eflag = true;
        break;
      case 'p':
        //E scheduler decision
        pflag = true;
        break;
      case 's':
        //scheduler type
        sscanf(optarg, "%c %d:%d", &scheduler_type, &t_quantum, &maxprio);
        break;
      case '?':
        printf("error optopt: %c\n", optopt);
        printf("error opterr: %d\n", opterr);
        break;
      default:
        exit(EXIT_FAILURE);
        break;
    }
    // cout << "scheduler_type" << scheduler_type << "t_quantum" << t_quantum
    //      << "maxprio" << maxprio << endl;
  }
}

int main(int argc, char* argv[]){
  setbuf(stdout, NULL);
  process_cmd(argc, argv);

  // ReadFile
  string filename = argv[argc - 2];
  vector<Process> process_list;
  process_list = ReadFile(filename); 
  // ReadRfile
  string rfilename = argv[argc - 1];
  RandomNumberGenerator rand_num = RandomNumberGenerator(rfilename);

  //assign maxprio for all the process
  for (int i = 0; i < process_list.size(); i++){
    process_list[i].prio = rand_num.GetNext(maxprio);
    process_list[i].dynamic_prio = process_list[i].prio - 1;
  }

  // call DES layer
  DiscreteEventSimulator des = DiscreteEventSimulator(process_list);
  // des.PrintAllProcess();
  // des.PrintAllEvent();
  
  /*********************/
  // construct scheduler
  /*********************/
  Scheduler* Sched;
  switch (scheduler_type) {
  case 'F':
    Sched = new FCFSScheduler();
    scheduler_name = "FCFS";
    break;

  case 'L':
    Sched = new LCFSScheduler();
    scheduler_name = "LCFS";
    break;

  case 'S':
    Sched = new SRTFScheduler(process_list);
    scheduler_name = "SRTF";
    break;

  case 'R':
    Sched = new FCFSScheduler();
    scheduler_name = "RR " + to_string(t_quantum);
    break; 

  case 'P':
    Sched = new PRIOScheduler(process_list, maxprio);
    scheduler_name = "PRIO " + to_string(t_quantum);
    is_prio = true;
    break;

  case 'E':
    Sched = new PREPRIOScheduler(process_list, maxprio);
    scheduler_name = "PREPRIO " + to_string(t_quantum);
    is_prio = true;
    break; 

  default:
    break;
  }
  
  // at the beginning no process is running 
  int CURRENT_RUNNING_PROCESS = -1;
  int TOTAL_IO_PROCESS = 0;
  int io_processing_time = 0;
  int current_time = 0;

  Event* evt;
  evt = des.GetEvent(); 
  while (evt != NULL){
    int pros = evt->pid;
    int prev_time = current_time;
    current_time = evt->time_stamp;
    int time_in_prev_state = current_time 
                             - process_list[pros].curr_state_timestamp;
    int prev_timestamp = process_list[pros].curr_state_timestamp;
    ProcessState old_state = evt->old_state;
    ProcessState new_state = evt->new_state;
    bool CALL_SCHEDULER = false;

    int cb = process_list[pros].cpu_burst;
    int rem = process_list[pros].time_left_cpu;
    int time_left_cpu_burst = process_list[pros].time_left_cpu_burst;
    int prio = process_list[pros].prio; 
    int dynamicPrio = process_list[pros].dynamic_prio;
    
    // When IO_QUEUE IS NOT EMPTY add IO Time
    if (TOTAL_IO_PROCESS > 0){
      io_processing_time = io_processing_time + (current_time - prev_time);
    }
    /*************************/
    // new_state == READY
    /*************************/
    if (new_state == READY){
      if (vflag){
        cout << current_time << " " << pros << " " << time_in_prev_state << ": "
             << old_state << " -> " << new_state;
      }

      /**************************************************************/
      // from block and from running need to deduct the dynamic_prio
      /**************************************************************/
      // old_state == BLOCKED
      if (old_state == BLOCK){
        TOTAL_IO_PROCESS -= 1;
        if (is_prio){
          process_list[pros].dynamic_prio = prio - 1;
        }
      }
      // old_state == RUNNING
      if (old_state == RUNNING){
        if (is_prio){
          process_list[pros].dynamic_prio -= 1;
        }
      }

      // preempt dealing
      if (Sched->test_preempt(pros) == true && CURRENT_RUNNING_PROCESS != -1
          && pros != CURRENT_RUNNING_PROCESS){
        if (process_list[pros].dynamic_prio > process_list[CURRENT_RUNNING_PROCESS].dynamic_prio){
          if ((process_list[CURRENT_RUNNING_PROCESS].curr_state_timestamp +
              process_list[CURRENT_RUNNING_PROCESS].time_in_curr_state) > current_time){
            if (vflag){
              cout << endl;
              cout << "---> PRIO preemption " << CURRENT_RUNNING_PROCESS << " by " << pros 
                  << " ? " << "1" << " TS=" << (process_list[CURRENT_RUNNING_PROCESS].curr_state_timestamp +
                      process_list[CURRENT_RUNNING_PROCESS].time_in_curr_state)
                  << " now=" << current_time <<") --> YES";
            }
            // remove the preempted event
            int removed_event_timestamp = des.CancelEvent(CURRENT_RUNNING_PROCESS);
            int renew_cpu_burst = removed_event_timestamp - current_time;
            process_list[CURRENT_RUNNING_PROCESS].time_left_cpu += removed_event_timestamp - current_time;
            process_list[CURRENT_RUNNING_PROCESS].time_in_curr_state = 
                current_time - process_list[CURRENT_RUNNING_PROCESS].curr_state_timestamp;
            // add the event back to event queue
            Event event;
            event.pid = CURRENT_RUNNING_PROCESS;
            event.new_state = READY;
            event.old_state = RUNNING;
            event.time_stamp = current_time;
            des.AddEvent(event);
            // CURRENT_RUNNING_PROCESS = -1;
          } else{
              if (vflag){
                cout << endl;
                cout << "---> PRIO preemption " << CURRENT_RUNNING_PROCESS << " by " << pros 
                    << " ? " << "1" << " TS=" << (process_list[CURRENT_RUNNING_PROCESS].curr_state_timestamp +
                        process_list[CURRENT_RUNNING_PROCESS].time_in_curr_state)
                    << " now=" << current_time <<") --> NO";
              }
          }
        } else{
          if (vflag){
                  cout << endl;
                  cout << "---> PRIO preemption " << CURRENT_RUNNING_PROCESS << " by " << pros 
                      << " ? " << "0" << " TS=" << (process_list[CURRENT_RUNNING_PROCESS].curr_state_timestamp +
                          process_list[CURRENT_RUNNING_PROCESS].time_in_curr_state)
                      << " now=" << current_time <<") --> NO";
          }
        }
      }
      // old_state == RUNNING
      if (old_state == RUNNING){
        process_list[pros].time_left_cpu_burst -= min(process_list[pros].time_in_curr_state, t_quantum);
        cb = process_list[pros].time_left_cpu_burst;
        CURRENT_RUNNING_PROCESS = -1;       
        if (vflag){
          cout << "  cb=" << cb << " rem=" << rem
               << " prio=" << dynamicPrio; 
        }
      }
      // update process to the process_list
      // assume there is no cpu wait time in run queue
      // i.e. in the "time in prev state" of ready -> running "time_in_curr_state" may be false
      // for calculating wait time using variable "time_in_prev_state"
      process_list[pros].time_in_curr_state = 0;
      process_list[pros].curr_state_timestamp = current_time;
      // must add to run queue
      Sched->add_process(pros);
      if (vflag == true){
        cout << endl; 
      }     
      CALL_SCHEDULER = true; // conditional on whether something is run
    }
    /*************************/
    // new_state == RUNNING
    /*************************/
    if (new_state == RUNNING){
      // ready -> running 
      // if time_left_cpu_burst = 0 -> get cpu burst time   
      if (time_left_cpu_burst == 0){
        cb = min(rand_num.GetNext(process_list[pros].cpu_burst),
                 process_list[pros].time_left_cpu);
        // cout << "cb_getnext" << cb << endl;
        process_list[pros].time_left_cpu_burst = cb;
      } else{
        cb = time_left_cpu_burst;
      }
      bool is_done = false;
      // need to update cpu wait time first
      int cpu_wait_time = time_in_prev_state;
      process_list[pros].cpu_wait_time += cpu_wait_time;
      if (cb >= rem && cb <= t_quantum){
        cb = rem;
        is_done = true;
      }
      if (vflag){
        cout << current_time << " " << pros << " " << cpu_wait_time << ": "
             << old_state << " -> " << new_state;

        cout << " cb=" << cb << " rem=" << rem
             << " prio=" << dynamicPrio << endl;
      }
      // update process in process_list
      process_list[pros].time_left_cpu -= min(cb, t_quantum);
      process_list[pros].time_in_curr_state = min(cb, t_quantum);
      process_list[pros].curr_state_timestamp = current_time;

      // put new event into event queue to done/ to ready/ to block
      if (is_done){
        Event event;
        event.pid = pros;
        event.new_state = DONE;
        event.old_state = RUNNING;
        event.time_stamp = current_time + cb;
        des.AddEvent(event);
      }else{
        Event event;
        event.pid = pros;
        if (cb > t_quantum){
          event.new_state = READY;
        } else{
          event.new_state = BLOCK;
        }       
        event.old_state = RUNNING;
        event.time_stamp = current_time + min(cb, t_quantum);
        des.AddEvent(event);
      }
    }
    /*************************/
    // new_state == BLOCK
    /*************************/
    if (new_state == BLOCK){
      if (vflag){
        cout << current_time << " " << pros << " " << time_in_prev_state << ": "
             << old_state << " -> " << new_state;

        // cout << pros << " is at block" << endl;
      }
      // get io burst time
      int ib = rand_num.GetNext(process_list[pros].io_burst);
      // cout << "ib_getnext" << ib << endl;
      if (vflag){
        cout << "  ib=" << ib
              << " rem=" << rem << endl;
      }
      // update process in process_list
      process_list[pros].IO_total += ib;
      process_list[pros].time_in_curr_state = ib;
      process_list[pros].curr_state_timestamp = current_time;
      process_list[pros].time_left_cpu_burst = 0;
      // put running -> block into queue
      Event event;
      event.pid = pros;
      event.new_state = READY;
      event.old_state = BLOCK;
      event.time_stamp = current_time + ib;
      des.AddEvent(event);
      // call scheduler must be RUN -> BLOCK
      CALL_SCHEDULER = true;
      CURRENT_RUNNING_PROCESS = -1;
      TOTAL_IO_PROCESS += 1;
    }

    // /*************************/
    // //new_state == PREEMPTED
    // /*************************/
    // if (new_state == PREEMPTED){
    //   //call scheduler must be RUN -> PREEMPTED
    //   CALL_SCHEDULER = true;
    // }

    /*************************/
    // new_state == DONE
    /*************************/
    if (new_state == DONE){
      //running -> Done
      if (vflag){
        cout << current_time << " " << pros << " " << time_in_prev_state << ": "
             << new_state <<endl;
      }
      process_list[pros].finish_time = current_time;
      CURRENT_RUNNING_PROCESS = -1;
      CALL_SCHEDULER = true;
    }
    /******************************/
    // remove event from event queue
    /******************************/
    des.RemoveEvent(evt);

    /*************************/
    // CALL_SCHEDULER
    /*************************/
    if (CALL_SCHEDULER){
      if (des.GetEvent() != nullptr && des.GetEvent()->time_stamp == current_time){
        evt = des.GetEvent();
        // cout << endl;
        // int pros = evt->pid;
        // cout << "add_process_Sched" <<endl;
        // Sched->add_process(pros);
        // des.RemoveEvent(evt);
        continue;
      }
      CALL_SCHEDULER = false;
      if (CURRENT_RUNNING_PROCESS == -1){
        CURRENT_RUNNING_PROCESS = Sched->get_next_process();
        if (CURRENT_RUNNING_PROCESS == -1){
          evt = des.GetEvent();
          // cout << endl;
          continue;
        }
        // create event to make this process runnable for same time:
        Event event;
        event.pid = CURRENT_RUNNING_PROCESS;
        event.new_state = RUNNING;
        event.old_state = READY;
        event.time_stamp = current_time;
        des.AddEvent(event);
      }
    }
    // cout << "Event:";
    // cout << evt->pid << ":";
    // cout << evt->new_state << ":";
    // cout << evt->old_state << ":";
    // cout << evt->time_stamp << endl;
    // cout << endl;
    evt = des.GetEvent();
  }
  
  /****************************/
  // print scheduler summary
  /****************************/
  cout << scheduler_name << endl;

  int finish_time = current_time;
  double cpu_util_time = 0;
  double sum_turn_around_time = 0;
  double sum_wait_time = 0;
  double process_num = process_list.size();
  for (int i = 0; i < process_list.size(); i++){
    Process p = process_list[i];
    printf("%04d: %4d %4d %4d %4d %1d | %5d %5d %5d %5d\n", p.pid, p.arrival_time,
				p.total_cpu_time, p.cpu_burst, p.io_burst, p.prio, p.finish_time,
        p.finish_time - p.arrival_time, p.IO_total, p.cpu_wait_time);
    cpu_util_time += p.total_cpu_time;
    sum_turn_around_time += p.finish_time - p.arrival_time;
    sum_wait_time += p.cpu_wait_time;
  }

  printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n", 
      finish_time,
      cpu_util_time * 100 / (double) finish_time, 
      (double) io_processing_time * 100 / (double) finish_time,
      sum_turn_around_time / process_num, 
      sum_wait_time / process_num,
      process_num * 100 / (double) finish_time);

  return 0;
}
#ifndef SIMPLE_SCHEDULER_H
#define SIMPLE_SCHEDULER_H

#include <iostream>
#include <list>
#include <vector>

#include "scheduler.h"
#include "process.h"

using namespace std;

/***********/
//FCFS
/***********/
class FCFSScheduler : public Scheduler {
  public:
    void add_process(int pros);
    int get_next_process();
    void PrintAllEvent();
    bool test_preempt(int pros); 
  private:
    std::list<int> run_queue_;
};

/***********/
//LCFS
/***********/
class LCFSScheduler : public Scheduler {
  public:
    void add_process(int pros);
    int get_next_process();
    void PrintAllEvent();
    bool test_preempt(int pros); 
  private:
    std::list<int> run_queue_;
};

/***********/
//SRTF
/***********/
class SRTFScheduler : public Scheduler {
  public:
    void add_process(int pros);
    int get_next_process();
    void PrintAllEvent();
    bool test_preempt(int pros); 
    const std::vector<Process>& process_list_;
    SRTFScheduler(const std::vector<Process>& process_list) 
        : process_list_(process_list) {}
  private:
    std::list<int> run_queue_;
};

/*******************/
//RR equal to FCFS
/*******************/

/***********/
//PRIO
/***********/
class PRIOScheduler : public Scheduler {
  public:
    explicit PRIOScheduler(std::vector<Process>& process_list, int maxprio) 
        : process_list_(process_list),
          active_queue_(new vector<list<int>>(maxprio)),
          expire_queue_(new vector<list<int>>(maxprio)){};

    void add_process(int pros);
    int get_next_process();
    void PrintAllEvent();
    bool test_preempt(int pros); 
  
  protected:
    std::vector<Process>& process_list_;
    vector<list<int>>* active_queue_;
    vector<list<int>>* expire_queue_;
};

/***********/
//PREPRIO
/***********/
class PREPRIOScheduler : public PRIOScheduler {
  public:
    using PRIOScheduler::PRIOScheduler;
    bool test_preempt(int pros);
};

#endif  // SIMPLE_SCHEDULER_H

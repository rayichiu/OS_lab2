#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <list>

class Scheduler {
  public:
    virtual void add_process(int pros) = 0;
    virtual int get_next_process() = 0;
    virtual void PrintAllEvent() = 0;
    virtual bool test_preempt(int pros) = 0; // false but for ‘E’
    // tests whether new ready process should preeempt running
};


#endif  // SCHEDULER_H
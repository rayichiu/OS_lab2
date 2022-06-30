#ifndef DES_H
#define DES_H

#include <iostream>
#include <list>
#include <vector>

#include "process.h"

class DiscreteEventSimulator{
  public:
    DiscreteEventSimulator(std::vector<Process> processes);

    // Get the next event to execute
    Event* GetEvent();
    // Add an event to the event queue
    void AddEvent(Event event);
    // remove an event from the event queue
    void RemoveEvent(Event* event);
    // in preempt
    int CancelEvent(int pros);

    void PrintAllProcess();
    void PrintAllEvent();

  private:
    std::list<Event> event_queue_;
    std::vector<Process> processes_;    

};
#endif  // DES_H

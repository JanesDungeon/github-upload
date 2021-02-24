#include "TrafficLight.h"
#include <iostream>
#include <random>

#include <chrono>
#include <memory>

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::Receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait()
    // to wait for and receive new messages and pull them from the queue using move semantics.
    // The received object should then be returned by the receive function.
    std::unique_lock<std::mutex> ulck(_mtx);
    _cond.wait(ulck, [this]{ return !_queue.empty(); });
    TrafficLightPhase newMsg = std::move(_queue.front());
    _queue.pop_front();
    return newMsg;
}

template <typename T>
void MessageQueue<T>::Send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex>
    // as well as _condition.notify_one() to add a new message to the queue and
    //  afterwards send a notification
    
    // to detect runtime bugs
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    try
    {
        std::lock_guard<std::mutex> lckGuard(_mtx);
        std::cout << "Adding message " << msg << " to MessageQueue\n";
        _queue.push_back(std::move(msg));
        _cond.notify_one();

    }
    catch(const std::exception& e)
        {
        std::cerr << e.what() << '\n';
    }
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop
    // runs and repeatedly calls the receive function on the message queue.
    // Once it receives TrafficLightPhase::green, the method returns.

    while(true)
    {   
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if(_messageQueue.Receive() == TrafficLightPhase::green)
        {
            return ;
        }
    }
}



void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be
//started in a thread when the public method „simulate“ is called. To do this, use
//the thread queue in the base class.
threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread 

void TrafficLight::cycleThroughPhases() {
  // FP.2a : Implement the function with an infinite loop that measures the time
  // between two loop cycles and toggles the current phase of the traffic light
  // between red and green and sends an update method to the message queue using
  // move semantics.The cycle duration should be a random value between 4 & 6
  // seconds. Also, the while-loop should use std::this_thread::sleep_for to
  // wait 1ms between two cycles.

  int randomTime = rand() % 2000 + 4000; // 4000 - 6000 ms
  auto begin = std::chrono::high_resolution_clock::now();

  while (true) 
  { 
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);

    
    if (elapsed.count() == randomTime) // toggle every 4-6 seconds
    { 
      if (getCurrentPhase() == green) {
        setCurrentPhase(red);
        _messageQueue.Send(std::move(red));
      } else {
        setCurrentPhase(green);
        _messageQueue.Send(std::move(green));
      }
    //new random time
    randomTime = rand() % 2000 + 4000;
    begin = std::chrono::high_resolution_clock::now();//reset cycle time
    }
  }
}

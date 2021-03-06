#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include "TrafficObject.h"
#include <condition_variable>
#include <deque>
#include <mutex>

class Vehicle;
template <class T> class MessageQueue;

enum class TrafficLightPhase { red, green };

template <class T> class MessageQueue {
public:
  void Send(T &&msg);
  T Receive();

private:
  std::condition_variable _cond;
  std::mutex _mtx;
  std::deque<TrafficLightPhase> _queue;
};



class TrafficLight : public TrafficObject {
public:
  TrafficLight();

  TrafficLightPhase getCurrentPhase() { return _currentPhase; }
  void setCurrentPhase(TrafficLightPhase phase) { _currentPhase = phase; }

  void simulate() override;
  void waitForGreen();
  void cycleThroughPhases();

private:

  MessageQueue<TrafficLightPhase> _messageQueue;
  TrafficLightPhase _currentPhase;
};

#endif
#include "TrafficLight.h"
#include <iostream>
#include <random>

#include <chrono>
#include <memory>


template <typename T> T MessageQueue<T>::Receive() {
  
  std::unique_lock<std::mutex> ulck(_mtx);

  _cond.wait(ulck, [this] { return !_queue.empty(); });
  TrafficLightPhase newMsg = std::move(_queue.front());
  _queue.pop_front();
  return newMsg;
}

template <typename T> void MessageQueue<T>::Send(T &&msg) {

  try {
    std::lock_guard<std::mutex> lckGuard(_mtx);
    _queue.emplace_back(std::move(msg));
    _cond.notify_one();

  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
  }
}


TrafficLight::TrafficLight() { _currentPhase = TrafficLightPhase::red; }

void TrafficLight::waitForGreen() {

  while (true) {
    TrafficLightPhase phase = _messageQueue.Receive();
    if (phase == TrafficLightPhase::green) {
      return;
    }
  }
}

void TrafficLight::simulate() {
  threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}


void TrafficLight::cycleThroughPhases() {

  int randomTime = rand() % 2000 + 4000; // ms
  auto begin = std::chrono::high_resolution_clock::now();

  while (true) {
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);

    if (elapsed.count() >= randomTime) // toggle every 4-6 seconds
    {
      if (getCurrentPhase() == TrafficLightPhase::green) {
        setCurrentPhase(TrafficLightPhase::red);
        _messageQueue.Send(std::move(_currentPhase));
      } else {
        setCurrentPhase(TrafficLightPhase::green);
        _messageQueue.Send(std::move(_currentPhase));
      }
      randomTime = rand() % 2000 + 4000;
      begin = std::chrono::high_resolution_clock::now(); 
    }

  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

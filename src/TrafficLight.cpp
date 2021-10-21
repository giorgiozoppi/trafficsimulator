#include "TrafficLight.h"
#include <iostream>
#include <random>

/** Implementation of the RandomGenerator */
RandomGenerator::RandomGenerator() {
  _eng = std::mt19937(_device());
  _distribution = std::uniform_int_distribution<>(4, 6);
}
int RandomGenerator::nextInt() { return _distribution(_eng); }
/* Implementation of class "MessageQueue" */
/*
 */

template <typename T> T MessageQueue<T>::receive() {

  std::unique_lock<std::mutex> lk(_mutex);
  /// here we use this and queue to avoid
  /// spurious wake-up.
  _new_item.wait(lk, [this] { return !_queue.empty(); });

  T item = std::move(_queue.front());
  _queue.pop_front();
  return item;
}
template <typename T> void MessageQueue<T>::send(T &&msg) {
  std::unique_lock<std::mutex> lk(_mutex);
  _queue.clear();
  _queue.emplace_back(std::move(msg));
  // Manual unlocking is done before notifying, to avoid waking up
  // the waiting thread only to block again - see cppreference.com
  lk.unlock();
  _new_item.notify_one();
}
void TrafficLight::waitForGreen() {
  TrafficLightPhase shape;
  do {
    shape = _queue.receive();
  } while (shape != TrafficLightPhase::green);
}
TrafficLight::TrafficLight() { _currentPhase = TrafficLightPhase::red; }
TrafficLight::~TrafficLight() {}

TrafficLightPhase TrafficLight::getCurrentPhase() const {
  return _currentPhase;
}

void TrafficLight::cycleThroughPhases() {

  auto previous{std::chrono::high_resolution_clock::now()};
  auto interval = _generator.nextInt() * 1000;
  auto current{std::chrono::high_resolution_clock::now()};

  while (true) {

    auto seconds = std::chrono::duration_cast<std::chrono::milliseconds>(
        current - previous);

    // we switch when the counts are between 4 and 6 seconds.
    if (seconds.count() >= interval) {
      if (_currentPhase == TrafficLightPhase::red) {
        _currentPhase = TrafficLightPhase::green;
      } else if (_currentPhase == TrafficLightPhase::green) {
        _currentPhase = TrafficLightPhase::red;
      }
      previous = current;
      current = std::chrono::high_resolution_clock::now();
      _queue.send(std::move(_currentPhase));
    } else {
      current = std::chrono::high_resolution_clock::now();
    }
    // required change: Sleep shall happen each iteration so shall be placed out of the if statement
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}
void TrafficLight::simulate() {
  std::thread t(&TrafficLight::cycleThroughPhases, this);
  // FP.2b : Finally, the private method „cycleThroughPhases“ should be
  // started in a thread when the public method „simulate“ is called. To do
  // this, use the thread queue in the base class.
  threads.emplace_back(std::move(t));
}

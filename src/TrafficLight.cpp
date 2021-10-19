#include "TrafficLight.h"
#include <iostream>
#include <random>

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
  {
    std::lock_guard<std::mutex> lk(_mutex);
    _queue.emplace_back(std::move(msg));
  }
  _new_item.notify_one();
}

void TrafficLight::waitForGreen() {
  auto shape = TrafficLightPhase::red;
  while (shape != TrafficLightPhase::green) {
    shape = _queue.receive();
  }
}
TrafficLight::TrafficLight() {
  std::lock_guard<std::mutex> lk(_phase_mutex);
  _currentPhase = TrafficLightPhase::red;
}

TrafficLightPhase TrafficLight::getCurrentPhase() const {
  std::lock_guard<std::mutex> lk(_phase_mutex);
  return _currentPhase;
}
void TrafficLight::switchTrafficLightPhase() {
  std::unique_lock<std::mutex> lk(_phase_mutex);
  if (_currentPhase == TrafficLightPhase::red) {
    _currentPhase = TrafficLightPhase::green;
    lk.unlock();
    TrafficLightPhase state{TrafficLightPhase::green};
    _queue.send(std::move(state));
    lk.lock();
  } else if (_currentPhase == TrafficLightPhase::green) {
    _currentPhase = TrafficLightPhase::red;
  }
}

void TrafficLight::cycleThroughPhases() {

  std::random_device rd;
  std::mt19937 eng(rd());
  std::uniform_int_distribution<> distr(4, 6);

  auto previous{std::chrono::high_resolution_clock::now()};

  while (!_stopped) {
    auto current{std::chrono::high_resolution_clock::now()};

    auto seconds =
        std::chrono::duration_cast<std::chrono::seconds>(current - previous);

    if (seconds.count() >= distr(eng)) {
      switchTrafficLightPhase();
      previous = current;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  }
  _exited = true;
}
void TrafficLight::stop() {
  _stopped = true;
  auto free = TrafficLightPhase::green;
  while (!_exited) {
    _queue.send(std::move(free));
    _currentPhase = TrafficLightPhase::green;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}
void TrafficLight::simulate() {
  std::thread t(&TrafficLight::cycleThroughPhases, this);
  // FP.2b : Finally, the private method „cycleThroughPhases“ should be
  // started in a thread when the public method „simulate“ is called. To do
  // this, use the thread queue in the base class.
  threads.emplace_back(std::move(t));
}

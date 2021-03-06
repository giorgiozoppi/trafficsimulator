#include <chrono>
#include <future>
#include <iostream>
#include <random>
#include <thread>

#include "Intersection.h"
#include "Street.h"
#include "Vehicle.h"

/* Implementation of class "WaitingVehicles" */

int WaitingVehicles::getSize() {
  std::lock_guard<std::mutex> lock(_mutex);

  return _vehicles.size();
}

void WaitingVehicles::pushBack(std::shared_ptr<Vehicle> vehicle,
                               std::promise<void> &&promise) {
  std::lock_guard<std::mutex> lock(_mutex);

  _vehicles.push_back(vehicle);
  _promises.push_back(std::move(promise));
}

void WaitingVehicles::permitEntryToFirstInQueue() {
  std::lock_guard<std::mutex> lock(_mutex);

  // get entries from the front of both queues
  auto firstPromise = _promises.begin();
  auto firstVehicle = _vehicles.begin();

  // fulfill promise and send signal back that permission to enter has been
  // granted
  firstPromise->set_value();

  // remove front elements from both queues
  _vehicles.erase(firstVehicle);
  _promises.erase(firstPromise);
}

/* Implementation of class "Intersection" */

Intersection::Intersection() {
  _type = ObjectType::objectIntersection;
  _isBlocked = false;
}
Intersection::~Intersection() {}

void Intersection::addStreet(std::shared_ptr<Street> street) {
  _streets.push_back(street);
}

std::vector<std::shared_ptr<Street>>
Intersection::queryStreets(std::shared_ptr<Street> incoming) {
  // store all outgoing streets in a vector ...
  std::vector<std::shared_ptr<Street>> outgoings;
  for (auto it : _streets) {
    if (incoming->getID() !=
        it->getID()) // ... except the street making the inquiry
    {
      outgoings.push_back(it);
    }
  }

  return outgoings;
}

// adds a new vehicle to the queue and returns once the vehicle is allowed to
// enter
void Intersection::addVehicleToQueue(std::shared_ptr<Vehicle> vehicle) {
  std::unique_lock<std::mutex> lck(_mtx);
  std::cout << "Intersection #" << _id
            << "::addVehicleToQueue: thread id = " << std::this_thread::get_id()
            << std::endl;
  lck.unlock();

  // add new vehicle to the end of the waiting line
  std::promise<void> prmsVehicleAllowedToEnter;
  std::future<void> ftrVehicleAllowedToEnter =
      prmsVehicleAllowedToEnter.get_future();
  _waitingVehicles.pushBack(vehicle, std::move(prmsVehicleAllowedToEnter));

  // wait until the vehicle is allowed to enter
  ftrVehicleAllowedToEnter.wait();
  lck.lock();

  // FP.6b : use the methods TrafficLight::getCurrentPhase and
  // TrafficLight::waitForGreen to block the execution until the traffic light
  // turns green.
  // the test should be in mutual exclusion and it might be skipped but specifications
  // tell us to use getCurrentPhase
  while (_trafficLight.getCurrentPhase() == TrafficLightPhase::red) {
    lck.unlock();
    //    This block of code is not considered to be a shared resource so it
    //    could be easily out of the lock guard
    _trafficLight.waitForGreen();
    lck.lock();
  }
  std::cout << "Intersection #" << _id << ": Vehicle #" << vehicle->getID()
            << " is granted entry." << std::endl;

  lck.unlock();
}

void Intersection::vehicleHasLeft(std::shared_ptr<Vehicle> vehicle) {
  // unblock queue processing
  std::cout << "Intersection #" << _id << ": Vehicle #" << vehicle->getID()
            << " is granted entry." << std::endl;

  this->setIsBlocked(false);
}

void Intersection::setIsBlocked(bool isBlocked) {
  // note that blocked is a std::atomic.
  // might be happend that a vehicle checks before set.
  _isBlocked = isBlocked;
  std::cout << "Intersection #" << _id << " isBlocked=" << isBlocked
            << std::endl;
}

// virtual function which is executed in a thread
void Intersection::simulate() // using threads + promises/futures + exceptions
{
  // FP.6a : In Intersection.h, add a private member _trafficLight of type
  // TrafficLight. At this position, start the simulation of _trafficLight.
  _trafficLight.simulate();
  // launch vehicle queue processing in a thread
  threads.emplace_back(std::thread(&Intersection::processVehicleQueue, this));
}

void Intersection::processVehicleQueue() {

  // continuously process the vehicle queue
  while (true) {
    // sleep at every iteration to reduce CPU usage
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // only proceed when at least one vehicle is waiting in the queue
    if (_waitingVehicles.getSize() > 0 && !_isBlocked) {
      // set intersection to "blocked" to prevent other vehicles from entering
      this->setIsBlocked(true);

      // permit entry to first vehicle in the queue (FIFO)
      _waitingVehicles.permitEntryToFirstInQueue();
    }
  }
}
bool Intersection::trafficLightIsGreen() {
  return (_trafficLight.getCurrentPhase() == TrafficLightPhase::green);
}
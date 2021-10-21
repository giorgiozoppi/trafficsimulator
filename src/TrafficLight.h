#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include <mutex>
#include <deque>
#include <random>
#include <condition_variable>
#include <utility>
#include "TrafficObject.h"

// forward declarations to avoid include cycle
class Vehicle;

// FP.3 Define a class „MessageQueue“ which has the public methods send and receive.
// Send should take an rvalue reference of type TrafficLightPhase whereas receive should return this type.
// Also, the class should define an std::dequeue called _queue, which stores objects of type TrafficLightPhase.
// Also, there should be an std::condition_variable as well as an std::mutex as private members.

template <class T>
class MessageQueue
{
public:
    void send(T &&item);
    T receive();

private:
    std::deque<T> _queue;
    std::mutex _mutex;
    std::condition_variable _new_item;
};

// FP.1 : Define a class „TrafficLight“ which is a child class of TrafficObject.
// The class shall have the public methods „void waitForGreen()“ and „void simulate()“
// as well as „TrafficLightPhase getCurrentPhase()“, where TrafficLightPhase is an enum that
// can be either „red“ or „green“. Also, add the private method „void cycleThroughPhases()“.
// Furthermore, there shall be the private member _currentPhase which can take „red“ or „green“ as its value.

enum class TrafficLightPhase
{
    red,
    green
};
// RandomGenetor is a random number generator
// for the elapsed time of the semaphore.
struct RandomGenerator
{
    RandomGenerator();
    int nextInt();

private:
    std::random_device _device;
    std::uniform_int_distribution<> _distribution;
    std::mt19937 _eng;
};

class TrafficLight : public TrafficObject
{
public:
    // constructor / desctructor
    TrafficLight();
    ~TrafficLight();
    // I delete the copy due to condition/mutex
    // some compilers deal with this automagically removing the copy/move
    // since we've deleted the copy constructor the object
    // is not movable or copyable.

    TrafficLight(const TrafficLight &light) = delete;
    TrafficLight &operator=(const TrafficLight &light) = delete;

    void waitForGreen();
    void simulate() override;

    // getters / setters
    TrafficLightPhase getCurrentPhase() const;
    // typical behaviour methods

private:
    // typical behaviour methods
    void cycleThroughPhases();
    // FP.4b : create a private member of type MessageQueue for messages of type TrafficLightPhase
    // and use it within the infinite loop to push each new TrafficLightPhase into it by calling
    // send in conjunction with move semantics.
    std::condition_variable _condition;
    std::mutex _mutex;
    TrafficLightPhase _currentPhase;
    // we add a generator foreach TrafficLight.
    // we might want to avoid singletons.
    // this mitigate the effect of generation engine creation since we create the traffic light before executing
    // the animation, and we might want that each traffic light has its own different
    // random period.
    RandomGenerator _generator;
    MessageQueue<TrafficLightPhase> _queue;
};

#endif
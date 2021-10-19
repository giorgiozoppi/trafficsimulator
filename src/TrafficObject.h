#ifndef TRAFFICOBJECT_H
#define TRAFFICOBJECT_H

#include <vector>
#include <thread>
#include <mutex>
#include <atomic>

enum ObjectType
{
    noObject,
    objectVehicle,
    objectIntersection,
    objectStreet,
};

class TrafficObject
{
public:
    // constructor / desctructor
    TrafficObject();
    virtual ~TrafficObject();

    // getter and setter
    int getID() { return _id; }
    void setPosition(double x, double y);
    void getPosition(double &x, double &y);
    ObjectType getType() { return _type; }

    // typical behaviour methods
    virtual void simulate(){};

protected:
    ObjectType _type;                 // identifies the class type
    int _id;                          // every traffic object has its own unique id
    double _posX, _posY;              // vehicle position in pixels
    // position mutex needs to protected 
    // it's clear if you use thread sanitizer.
    std::mutex _position_mutex;
    std::vector<std::thread> threads; // holds all threads that have been launched within this object

    static std::mutex _mtx;           // mutex shared by all traffic objects for protecting cout 
    
    /* stop and signaling mechanism
     * we loop until not _stopped and when we busy wait before _exited
     * when we're out of the loop we set _exited.
     */

    std::atomic<bool> _stopped{false};
    std::atomic<bool> _exited{false};
    
private:
    static int _idCnt; // global variable for counting object ids
};

#endif
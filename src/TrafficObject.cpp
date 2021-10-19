#include "TrafficObject.h"
#include <algorithm>
#include <chrono>
#include <iostream>

// init static variable
int TrafficObject::_idCnt = 0;

std::mutex TrafficObject::_mtx;

void TrafficObject::setPosition(double x, double y) {
  // thread sanitizer indicates that it's needed during creation
  std::lock_guard<std::mutex> lk(_position_mutex);
  _posX = x;
  _posY = y;
}

void TrafficObject::getPosition(double &x, double &y) {
  // yes there were a race for whom was intended this app.
  std::lock_guard<std::mutex> lk(_position_mutex);
  x = _posX;
  y = _posY;
}

TrafficObject::TrafficObject() {
  _type = ObjectType::noObject;
  _id = _idCnt++;
}

TrafficObject::~TrafficObject() {
  // set up thread barrier before this object is destroyed
  std::for_each(threads.begin(), threads.end(),
                [](std::thread &t) { t.join(); });
}

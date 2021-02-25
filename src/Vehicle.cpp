#include "Vehicle.h"
#include "Intersection.h"
#include "Street.h"
#include <iostream>
#include <random>

Vehicle::Vehicle() {
  _currStreet = nullptr;
  _posStreet = 0.0;
  _type = ObjectType::objectVehicle;
  _speed = 400; // m/s
}

void Vehicle::setCurrentDestination(std::shared_ptr<Intersection> destination) {
  _currDestination = destination;

  // reset simulation parameters
  _posStreet = 0.0;
}

void Vehicle::simulate() {
  threads.emplace_back(std::thread(&Vehicle::drive, this));
}

void Vehicle::drive() {
  std::unique_lock<std::mutex> lck(_mtx);
  std::cout << "Vehicle #" << _id
            << "::drive: thread id = " << std::this_thread::get_id()
            << std::endl;
  lck.unlock();

  bool hasEnteredIntersection = false;
  double cycleDuration = 1; 
  std::chrono::time_point<std::chrono::system_clock> lastUpdate;

  lastUpdate = std::chrono::system_clock::now();
  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    long timeSinceLastUpdate =
        std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::system_clock::now() - lastUpdate).count();
    if (timeSinceLastUpdate >= cycleDuration) {
      // update position with a constant velocity motion model
      _posStreet += _speed * timeSinceLastUpdate / 1000;

      double completion = _posStreet / _currStreet->getLength();

      // compute current pixel position on street based on driving direction
      std::shared_ptr<Intersection> i1, i2;
      i2 = _currDestination;
      i1 = i2->getID() == _currStreet->getInIntersection()->getID()
               ? _currStreet->getOutIntersection()
               : _currStreet->getInIntersection();

      double x1, y1, x2, y2, xv, yv, dx, dy, l;
      i1->getPosition(x1, y1);
      i2->getPosition(x2, y2);
      dx = x2 - x1;
      dy = y2 - y1;
      l = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (x1 - x2));
      xv = x1 + completion * dx; 
      yv = y1 + completion * dy;
      this->setPosition(xv, yv);

      if (completion >= 0.9 && !hasEnteredIntersection) {
        auto ftrEntryGranted = std::async(&Intersection::addVehicleToQueue, _currDestination, get_shared_this());

        ftrEntryGranted.get();

        _speed /= 10.0;
        hasEnteredIntersection = true;
      }

      if (completion >= 1.0 && hasEnteredIntersection) {
        // choose next street and destination
        std::vector<std::shared_ptr<Street>> streetOptions = _currDestination->queryStreets(_currStreet);
        std::shared_ptr<Street> nextStreet;
        if (streetOptions.size() > 0) {
          std::random_device rd;
          std::mt19937 eng(rd());
          std::uniform_int_distribution<> distr(0, streetOptions.size() - 1);
          nextStreet = streetOptions.at(distr(eng));
        } else { 
          nextStreet = _currStreet;
        }

        // pick the one intersection at which the vehicle is currently not
        std::shared_ptr<Intersection> nextIntersection =
            nextStreet->getInIntersection()->getID() == _currDestination->getID() ? 
            nextStreet->getOutIntersection() : nextStreet->getInIntersection();

        _currDestination->vehicleHasLeft(get_shared_this());

        this->setCurrentDestination(nextIntersection);
        this->setCurrentStreet(nextStreet);

        _speed *= 10.0;
        hasEnteredIntersection = false;
      }

      lastUpdate = std::chrono::system_clock::now();
    }
  } 
}

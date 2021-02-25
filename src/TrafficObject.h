#ifndef TRAFFICOBJECT_H
#define TRAFFICOBJECT_H

#include <mutex>
#include <thread>
#include <vector>

enum ObjectType {
  noObject,
  objectVehicle,
  objectIntersection,
  objectStreet,
};

class TrafficObject {
public:
  TrafficObject();
  ~TrafficObject();

  int getID() { return _id; }
  void setPosition(double x, double y);
  void getPosition(double &x, double &y);
  ObjectType getType() { return _type; }

  virtual void simulate(){};

protected:
  ObjectType _type;    
  int _id;             
  double _posX, _posY; // vehicle position in pixels
  std::vector<std::thread> threads; 
  static std::mutex _mtx; 

private:
  static int _idCnt; 
};

#endif
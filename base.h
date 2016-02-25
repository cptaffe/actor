
#ifndef B_BASE_H_
#define B_BASE_H_

#include <string>

class Event {
public:
  virtual ~Event() {}
  // English language description of the event.
  virtual std::string Description() = 0;
};

class Actor {
public:
  virtual ~Actor() {}
  virtual void Handle(Event *e) = 0;
};

class Renderable {
public:
  virtual ~Renderable() {}
  virtual void Render() = 0;
};

#endif // B_BASE_H_

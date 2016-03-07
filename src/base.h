// Copyright 2016 Connor Taffe

#ifndef SRC_BASE_H_
#define SRC_BASE_H_

#include <memory>
#include <string>

class Event {
 public:
  virtual ~Event();
  // English language description of the event.
  virtual std::string Description() = 0;
};

class Actor {
 public:
  virtual ~Actor();
  virtual void Handle(std::shared_ptr<Event> const e) = 0;
};

#endif  // SRC_BASE_H_

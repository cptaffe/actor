// Copyright (c) 2016 Connor Taffe

#ifndef SRC_EVENTS_H_
#define SRC_EVENTS_H_

#include <memory>
#include <string>

#include "src/base.h"
#include "src/util.h"

// TODO(cptaffe): make events abstract classes for extending

namespace events {

class Terminate : public Event {
 public:
  explicit Terminate(std::string reason);
  std::string Description() override { return "Terminating: " + reason; }

 private:
  std::string reason;
};

class Say : public Event {
 public:
  Say(std::shared_ptr<Actor> actor, std::string message);
  std::string Description() override { return "Someone said something"; }
  std::string Message() { return message; }
  std::shared_ptr<Actor> Who() { return actor; }

 private:
  std::string message;
  std::shared_ptr<Actor> actor;
};

// Spawn an item
class Spawn : public Event {
 public:
  explicit Spawn(std::shared_ptr<class Actor> a);
  std::string Description() override { return "Spawning an actor"; }
  std::shared_ptr<class Actor> Actor() { return actor; }

 private:
  std::shared_ptr<class Actor> actor;
};

// delete an item
class Destroy : public Event {
 public:
  explicit Destroy(std::shared_ptr<class Actor> a);
  std::string Description() override { return "Destroying an actor"; }
  std::shared_ptr<class Actor> Actor() { return actor; }

 private:
  std::shared_ptr<class Actor> actor;
};

}  // namespace events

#endif  // SRC_EVENTS_H_

// Copyright (c) 2016 Connor Taffe

#ifndef SRC_EVENTS_H_
#define SRC_EVENTS_H_

#include <string>
#include "src/base.h"
#include "src/interfaces.h"
#include "src/util.h"

namespace events {

class Terminate : public Event, public interfaces::Terminal {
 public:
  explicit Terminate(std::string reason);
  std::string Description() override { return "Terminating: " + reason; }

 private:
  std::string reason;
};

class Say : public Event, public interfaces::Sayable {
 public:
  Say(std::shared_ptr<Actor> actor, std::string message);
  std::string Description() override { return "Someone said something"; }
  std::string Said() override { return message; }
  std::shared_ptr<Actor> Who() { return actor; }

 private:
  std::string message;
  std::shared_ptr<Actor> actor;
};

}  // namespace events

#endif  // SRC_EVENTS_H_

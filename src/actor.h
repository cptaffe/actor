// Copyright (c) 2016 Connor Taffe

#ifndef SRC_ACTOR_H_
#define SRC_ACTOR_H_

#include <chrono>
#include <iostream>
#include <random>
#include <sstream>
#include <string>

#include "src/base.h"
#include "src/events.h"
#include "src/interfaces.h"
#include "src/spool.h"
#include "src/util.h"

namespace actors {

class Sayer : public Actor {
 public:
  Sayer() : queue([=](std::string s) { std::cout << s << std::endl; }) {
    queue.Run(1);
  }
  ~Sayer() {
    queue.Kill();
    queue.Wait();
  }
  void Handle(std::shared_ptr<Event> e) override;

 private:
  util::ConsumerQueue<std::string> queue;
};

}  // namespace actors

#endif  // SRC_ACTOR_H_

// Copyright 2016 Connor Taffe

#ifndef SRC_SPOOL_H_
#define SRC_SPOOL_H_

#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "src/base.h"
#include "src/util.h"

// Event Spool singleton
class Spool : public Actor {
 public:
  static Spool *Instance() {
    if (instance == nullptr) {
      instance = new Spool();
    }
    return instance;
  }

  // Use the registered actors as receivers
  void Handle(std::shared_ptr<Event> e) override;

  // Specify receivers for an event
  void Handle(std::shared_ptr<Event> e,
              std::vector<std::shared_ptr<Actor>> ac) {
    std::vector<std::pair<std::shared_ptr<Actor>, std::shared_ptr<Event>>> v;
    for (auto a : ac) {
      v.push_back({a, e});
    }
    handles.Put(v);
  }

  void Run() { handles.Run(std::thread::hardware_concurrency()); }

  void Wait() { handles.Wait(); }

 private:
  Spool()
      : handles(
            [=](std::pair<std::shared_ptr<Actor>, std::shared_ptr<Event>> t) {
              t.first->Handle(t.second);
            }) {}
  Spool(Spool const &) = delete;
  Spool &operator=(Spool const &) = delete;
  static Spool *instance;
  std::mutex actorsMtx;
  std::set<std::shared_ptr<Actor>> actors;
  util::ConsumerQueue<std::pair<std::shared_ptr<Actor>, std::shared_ptr<Event>>>
      handles;
};

#endif  // SRC_SPOOL_H_

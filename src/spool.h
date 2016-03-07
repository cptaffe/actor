// Copyright 2016 Connor Taffe

#ifndef SRC_SPOOL_H_
#define SRC_SPOOL_H_

#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "src/base.h"
#include "src/interfaces.h"
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

  void Register(std::string id, std::shared_ptr<Actor> a) {
    std::unique_lock<std::mutex> lck(actorsMtx);
    actors.insert({id, a});
  }

  void Unregister(std::string id) {
    std::unique_lock<std::mutex> lck(actorsMtx);
    actors.erase(id);
  }

  void Unregister(std::shared_ptr<Actor> a) {
    for (auto i = actors.begin(); i != actors.end(); i++) {
      if (i->second == a) {
        actors.erase(i);
      }
    }
  }

  std::shared_ptr<Actor> ActorById(std::string id) {
    auto a = actors.find(id);
    if (a != actors.end()) {
      return a->second;
    }
    return nullptr;
  }

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
  std::map<std::string, std::shared_ptr<Actor>> actors;
  util::ConsumerQueue<std::pair<std::shared_ptr<Actor>, std::shared_ptr<Event>>>
      handles;
};

#endif  // SRC_SPOOL_H_

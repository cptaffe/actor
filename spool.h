
#ifndef B_SPOOL_H_
#define B_SPOOL_H_

#include "base.h"
#include "interfaces.h"
#include "util.h"
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

// Event Spool singleton
class Spool : public Actor {
public:
  static Spool *Instance() { return instance; }

  // Use the registered actors as receivers
  virtual void Handle(Event *e) override {
    std::vector<std::pair<Actor *, Event *>> v;
    for (auto a : ([&]() {
           std::unique_lock<std::mutex> lck(actorsMtx);
           std::vector<Actor *> v;
           for (auto a : actors) {
             v.push_back(a.second);
           }
           return v;
         })()) {
      v.push_back({a, e});
    }
    handles.Put(v);

    // Also handle the event
    ([=](interfaces::Terminal *t) {
      if (t != nullptr) {
        handles.Kill();
      }
    })(dynamic_cast<interfaces::Terminal *>(e));
  }

  // Specify receivers for an event
  void Handle(Event *e, std::vector<Actor *> ac) {
    std::vector<std::pair<Actor *, Event *>> v;
    for (auto a : ac) {
      v.push_back({a, e});
    }
    handles.Put(v);
  }

  void Run(int threads) { handles.Run(threads); }

  void Wait() { handles.Wait(); }

  void Register(std::string id, Actor *a) {
    std::unique_lock<std::mutex> lck(actorsMtx);
    actors.insert({id, a});
  }

  void Unregister(std::string id) {
    std::unique_lock<std::mutex> lck(actorsMtx);
    actors.erase(id);
  }

  void Unregister(Actor *a) {
    for (auto i = actors.begin(); i != actors.end(); i++) {
      if (i->second == a) {
        actors.erase(i);
      }
    }
  }

  Actor *ActorById(std::string id) {
    auto a = actors.find(id);
    if (a != actors.end()) {
      return a->second;
    }
    return nullptr;
  }

private:
  Spool()
      : handles([=](std::pair<Actor *, Event *> t) {
          t.first->Handle(t.second);
        }) {}
  Spool(Spool const &) = delete;
  Spool &operator=(Spool const &) = delete;
  static Spool *instance;
  std::mutex actorsMtx;
  std::map<std::string, Actor *> actors;
  ConsumerQueue<std::pair<Actor *, Event *>> handles;
};

#endif // B_SPOOL_H_

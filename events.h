
#ifndef B_EVENTS_H_
#define B_EVENTS_H_

#include "base.h"
#include "interfaces.h"
#include "util.h"
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

namespace events {

class Terminate : public Event {
public:
  Terminate(std::string s) : reason(s) {}
  virtual std::string Description() override {
    return "Terminating: " + reason;
  }

private:
  std::string reason;
};

// Event Spool singleton
class EventSpool : public Actor {
public:
  static EventSpool *Instance() { return instance; }

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
    ([=](Terminate *t) {
      if (t != nullptr) {
        handles.Kill();
      }
    })(dynamic_cast<Terminate *>(e));
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

  void RegisterActor(std::string id, Actor *a) {
    std::unique_lock<std::mutex> lck(actorsMtx);
    actors.insert({id, a});
  }

  Actor *ActorById(std::string id) {
    auto a = actors.find(id);
    if (a != actors.end()) {
      return a->second;
    }
    return nullptr;
  }

private:
  EventSpool()
      : handles([=](std::pair<Actor *, Event *> t) {
          t.first->Handle(t.second);
        }) {}
  EventSpool(EventSpool const &) = delete;
  EventSpool &operator=(EventSpool const &) = delete;
  static EventSpool *instance;
  std::mutex actorsMtx;
  std::map<std::string, Actor *> actors;
  ConsumerQueue<std::pair<Actor *, Event *>> handles;
};

class Attack : public Event, public interfaces::Attack {
public:
  Attack(Actor *f, Actor *t) : _attacker(f), _attackee(t) {}
  virtual std::string Description() override {
    return "Someone has attacked another";
  }
  virtual Actor *Attacker() override { return _attacker; }
  virtual Actor *Attackee() override { return _attackee; }

private:
  Actor *_attacker, *_attackee;
};

class Death : public Event, public interfaces::Death {
public:
  Death(Actor *d) : _deadActor(d) {}
  virtual std::string Description() override { return "Someone has died"; }
  virtual Actor *Who() override { return _deadActor; }

private:
  Actor *_deadActor;
};

class Say : public Event, public interfaces::Sayable {
public:
  Say(Actor *v, std::string s) : _saying(s), _voice(v) {}
  virtual std::string Description() override {
    return "Someone said something";
  }
  virtual std::string Said() override { return _saying; }
  virtual Actor *Who() { return _voice; }

private:
  std::string _saying;
  Actor *_voice;
};

class TaxEvent : public Event, public interfaces::TaxChange {
public:
  TaxEvent(double offset) : _offset(offset) {}
  virtual double Offset() override { return _offset; }
  virtual std::string Description() override {
    return "Someone changed the taxes";
  }

private:
  int _offset;
};

} // namespace events

#endif // B_EVENTS_H_

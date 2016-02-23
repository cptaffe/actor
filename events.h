
#ifndef B_EVENTS_H_
#define B_EVENTS_H_

#include "base.h"
#include "interfaces.h"
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

namespace events {

// Event Spool singleton
class EventSpool : public Actor {
public:
  static EventSpool *Instance() { return instance; }

  // TODO: thread-safe producer
  virtual void Handle(Event *e) override {
    std::unique_lock<std::mutex> lck(eventsMtx);
    events.push(e);
    eventsCv.notify_one();
  }

  void RegisterActor(Actor *a) {
    std::unique_lock<std::mutex> lck(actorsMtx);
    actors.push_back(a);
  }

  void Wait() { runThread.join(); }

private:
  EventSpool() {
    std::cout << "running thread" << std::endl;
    runThread = std::thread([&] {
      std::cout << "starting run" << std::endl;
      Instance()->Run();
    });
  }
  EventSpool(EventSpool const &) = delete;
  EventSpool &operator=(EventSpool const &) = delete;
  static EventSpool *instance;
  std::mutex actorsMtx;
  std::vector<Actor *> actors;
  std::mutex eventsMtx;
  std::condition_variable eventsCv;
  std::queue<Event *> events;

  std::thread runThread;

  // Event consumer
  void Run() {
    for (;;) {
      auto e = ([&]() {
        std::unique_lock<std::mutex> lck(eventsMtx);
        eventsCv.wait(lck, [&] { return !events.empty(); });
        auto e = events.front();
        events.pop();
        return e;
      })();

      std::unique_lock<std::mutex> lck(actorsMtx);
      for (auto a : actors) {
        a->Handle(e);
      }
    }
  }
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
  Say(std::string s) : _s(s) {}
  virtual std::string Description() override {
    return "Someone said something";
  }
  virtual std::string Said() override { return _s; }

private:
  std::string _s;
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

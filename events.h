
#ifndef B_EVENTS_H_
#define B_EVENTS_H_

#include "base.h"
#include "interfaces.h"
#include <condition_variable>
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

  // TODO: thread-safe producer
  virtual void Handle(Event *e) override {
    std::unique_lock<std::mutex> lck(eventsMtx);

    // Ignore events if terminated
    if (alive) {
      events.push(e);
      eventsCv.notify_one();
    }
  }

  void RegisterActor(std::string id, Actor *a) {
    std::unique_lock<std::mutex> lck(actorsMtx);
    actors.insert({id, a});
  }

  void Run(int threads) {
    for (int i = 0; i < threads; i++) {
      runThreads.push_back(new std::thread([&] { Instance()->Consume(); }));
    }
  }

  void Wait() {
    // Join worker threads
    for (auto t : runThreads) {
      t->join();
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
  EventSpool() {}
  EventSpool(EventSpool const &) = delete;
  EventSpool &operator=(EventSpool const &) = delete;
  static EventSpool *instance;
  std::mutex actorsMtx;
  std::map<std::string, Actor *> actors;
  std::mutex eventsMtx;
  std::condition_variable eventsCv;
  bool alive = true; // set to false once terminated
  std::queue<Event *> events;

  std::vector<std::thread *> runThreads;

  // Lock the actors and generate a list
  // of actors to call for this event.
  // NOTE: ordering of events is not guaranteed!
  void Relay(Event *e) {
    for (auto a : ([&]() {
           std::unique_lock<std::mutex> lck(actorsMtx);
           std::vector<Actor *> v;
           for (auto a : actors) {
             v.push_back(a.second);
           }
           return v;
         })()) {
      a->Handle(e);
    }
  }

  // Event consumer
  void Consume() {
    for (;;) {
      auto killAfterRelay = false;
      // Retrieve an event, releasing the lock immediately
      auto e = ([&]() -> Event * {
        std::unique_lock<std::mutex> lck(eventsMtx);
        eventsCv.wait(lck, [&] { return !events.empty() || !alive; });
        if (events.empty()) {
          // Dead and no events left to process
          return nullptr;
        }
        auto e = events.front();
        events.pop();

        // Event handler ends on encountering
        // a terminate event, but only after passing
        // the event to all listening actors,
        // and handling all generated events.
        ([&](Terminate *t) {
          if (t != nullptr) {
            killAfterRelay = true;
          }
        })(dynamic_cast<Terminate *>(e));

        return e;
      })();

      if (e == nullptr) {
        return; // stop consuming
      }
      Relay(e);
      delete e;

      // Terminates all consumers,
      // but allows for events to be added to the queue
      // in response to the Terminate response which will be processed.
      if (killAfterRelay) {
        alive = false;         // end queue
        eventsCv.notify_all(); // tell all consumers
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

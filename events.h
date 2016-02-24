
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

  // TODO: thread-safe producer
  virtual void Handle(Event *e) override { events.Put(e); }

  void RegisterActor(std::string id, Actor *a) {
    std::unique_lock<std::mutex> lck(actorsMtx);
    actors.insert({id, a});
  }

  void Run(int threads) { events.Run(threads); }

  void Wait() { events.Wait(); }

  Actor *ActorById(std::string id) {
    auto a = actors.find(id);
    if (a != actors.end()) {
      return a->second;
    }
    return nullptr;
  }

private:
  EventSpool() : events([=](Event *e) { Consume(e); }) {}
  EventSpool(EventSpool const &) = delete;
  EventSpool &operator=(EventSpool const &) = delete;
  static EventSpool *instance;
  std::mutex actorsMtx;
  std::map<std::string, Actor *> actors;
  ConsumerQueue<Event *> events;

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
  void Consume(Event *e) {
    auto killAfterRelay = false;
    // Event handler ends on encountering
    // a terminate event, but only after passing
    // the event to all listening actors,
    // and handling all generated events.
    ([&](Terminate *t) {
      if (t != nullptr) {
        killAfterRelay = true;
      }
    })(dynamic_cast<Terminate *>(e));
    Relay(e);
    delete e;

    // Terminates all consumers,
    // but allows for events to be added to the queue
    // in response to the Terminate response which will be processed.
    if (killAfterRelay) {
      events.Kill();
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

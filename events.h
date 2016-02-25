
#ifndef B_EVENTS_H_
#define B_EVENTS_H_

#include "base.h"
#include "interfaces.h"
#include "util.h"
#include <string>

namespace events {

class Terminate : public Event, public interfaces::Terminal {
public:
  Terminate(std::string s) : reason(s) {}
  virtual std::string Description() override {
    return "Terminating: " + reason;
  }

private:
  std::string reason;
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
  double _offset;
};

} // namespace events

#endif // B_EVENTS_H_

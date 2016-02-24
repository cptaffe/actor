
#ifndef ACTOR_H_
#define ACTOR_H_

#include <chrono>
#include <iostream>

#include "base.h"
#include "events.h"

namespace actors {
class Actor : public ::Actor {
protected:
  void Spawn(Event *e) { events::EventSpool::Instance()->Handle(e); }
};

class Npc : public actors::Actor {
public:
  virtual void Handle(Event *e) override {
    ([&](interfaces::Sayable *s) {
      if (s != nullptr) {
        if (s->Said() == "hey") {
          Spawn(new events::Say(this, "hi"));
        }
      }
    })(dynamic_cast<interfaces::Sayable *>(e));
  }
};

class King : public Actor {
public:
  virtual void Handle(Event *e) override {
    // Respond to an attack
    ([&](events::Say *s) {
      if (s != nullptr) {
        if (s->Who() ==
            events::EventSpool::Instance()->ActorById("innkeeper")) {
          Spawn(new events::Say(this, "Damn innkeeper!"));
        }
      }
    })(dynamic_cast<events::Say *>(e));
  }
};

class Sayer : public Actor {
public:
  virtual void Handle(Event *e) override {
    ([&](events::Say *s) {
      if (s != nullptr) {
        std::cout << s->Said() << std::endl;
      }
    })(dynamic_cast<events::Say *>(e));
  }
};

} // namespace actors

#endif // ACTOR_H_

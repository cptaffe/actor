
#ifndef ACTOR_H_
#define ACTOR_H_

#include <chrono>
#include <iostream>
#include <random>
#include <sstream>

#include "base.h"
#include "events.h"

namespace actors {
class Actor : public ::Actor {
protected:
  void Spawn(Event *e) { events::EventSpool::Instance()->Handle(e); }
  void Register(std::string id, ::Actor *a) {
    events::EventSpool::Instance()->RegisterActor(id, a);
  }
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

class Guard : public Actor {
  virtual void Handle(Event *e) override {
    // Respond to an attack
    ([&](events::Say *s) {
      if (s != nullptr) {
        if (s->Who() == events::EventSpool::Instance()->ActorById("king")) {
          Spawn(new events::Say(this, "Yeah! " + s->Said()));
        }
      }
    })(dynamic_cast<events::Say *>(e));
  }
};

class King : public Actor {
public:
  virtual void Handle(Event *e) override {
    ([&](events::Say *s) {
      if (s != nullptr) {
        if (s->Who() ==
            events::EventSpool::Instance()->ActorById("innkeeper")) {
          std::stringstream s;
          s << "guard-" << random(); // Random names so they don't clash
          Spawn(new events::Say(this, "Damn innkeeper!"));
          Register(s.str(), new Guard());
          Spawn(new events::Terminate("king is unhappy"));
        }
      }
    })(dynamic_cast<events::Say *>(e));
  }

private:
  std::mt19937_64 random;
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

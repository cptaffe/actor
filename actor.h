
#ifndef ACTOR_H_
#define ACTOR_H_

#include <iostream>

#include "base.h"
#include "events.h"

namespace actors {
class Actor : public ::Actor {
protected:
  void Say(std::string s) { Spawn(new events::Say(s)); }
  void Spawn(Event *e) { events::EventSpool::Instance()->Handle(e); }
};

class Npc : public actors::Actor {
public:
  virtual void Handle(Event *e) override {
    ([&](interfaces::TaxChange *t) {
      if (t != nullptr) {
        _kingApproval -= t->Offset() * taxChangeApprovalRatio;
      }
    })(dynamic_cast<interfaces::TaxChange *>(e));

    // Possible spawn other events
    if (_kingApproval <= static_cast<double>(Approval::kMurderousRage)) {
      // TODO: spawn an attack
    }
  }

private:
  enum class Approval : int {
    kMurderousRage = -1000,
    kDisdain = -100,
    kMeh = 0,
  };

  constexpr static const double taxChangeApprovalRatio = 1.0;
  double _kingApproval = 0;
};

class King : public Actor {
public:
  virtual void Handle(Event *e) override {
    // Respond to an attack
    ([&](events::Attack *a) {
      if (a != nullptr) {
        // TODO: respond to attack
      }
    })(dynamic_cast<events::Attack *>(e));
  }

private:
  constexpr static const int maxAssasinationAttempts = 2;
  int assasinationAttempts = 0;
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

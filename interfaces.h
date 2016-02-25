
#ifndef B_INTERFACES_H_
#define B_INTERFACES_H_

#include "base.h"

namespace interfaces {

// Terminal (exit loop)
class Terminal {
public:
  virtual ~Terminal() {}
};

// may be killed
class Killable {
public:
  virtual ~Killable() {}
  virtual void Die(Actor *) = 0;
};

// an attack
class Attack {
public:
  virtual ~Attack() {}
  virtual Actor *Attacker() = 0;
  virtual Actor *Attackee() = 0;
};

// a death occured
class Death {
public:
  virtual ~Death() {}
  virtual Actor *Who() = 0;
};

// can be said
class Sayable {
public:
  virtual ~Sayable() {}
  virtual std::string Said() = 0;
};

// change in taxes
class TaxChange {
public:
  virtual ~TaxChange() {}
  virtual double Offset() = 0;
};
} // namespace interfaces

#endif // B_INTERFACES_H_

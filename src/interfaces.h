// Copyright 2016 Connor Taffe

#ifndef SRC_INTERFACES_H_
#define SRC_INTERFACES_H_

#include <string>

#include "src/base.h"

namespace interfaces {

// Terminal (exit loop)
class Terminal {
 public:
  virtual ~Terminal();
};

// can be said
class Sayable {
 public:
  virtual ~Sayable();
  virtual std::string Said() = 0;
};

}  // namespace interfaces

#endif  // SRC_INTERFACES_H_

// Copyright 2016 Connor Taffe

#include "src/events.h"

namespace events {

Say::Say(std::shared_ptr<Actor> a, std::string m) : message{m}, actor{a} {}

Terminate::Terminate(std::string s) : reason{s} {}

}  // namespace events

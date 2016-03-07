// Copyright 2016 Connor Taffe

#include "src/actor.h"

namespace actors {

void Sayer::Handle(std::shared_ptr<Event> e) {
  ([&](std::shared_ptr<events::Say> s) {
    if (s != nullptr) {
      queue.Put(s->Message());
    }
  })(std::dynamic_pointer_cast<events::Say>(e));
}

}  // namespace actors

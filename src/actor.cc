// Copyright 2016 Connor Taffe

#include "src/actor.h"

namespace actors {

void Sayer::Handle(std::shared_ptr<Event> e) {
  ([&](events::Say *s) {
    if (s != nullptr) {
      queue.Put(s->Said());
    }
  })(dynamic_cast<events::Say *>(&(*e)));
}

}  // namespace actors

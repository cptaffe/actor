// Copyright 2016 Connor Taffe

#include "src/spool.h"
#include "src/events.h"

Spool *Spool::instance = nullptr;

void Spool::Handle(std::shared_ptr<Event> e) {
  std::vector<std::pair<std::shared_ptr<Actor>, std::shared_ptr<Event>>> v;
  for (auto a : actors) {
    v.push_back({a, e});
  }
  handles.Put(v);

  // Terminate spool
  ([=](std::shared_ptr<events::Terminate> t) {
    if (t != nullptr) {
      handles.Kill();
    }
  })(std::dynamic_pointer_cast<events::Terminate>(e));

  // Add a new actor
  ([=](std::shared_ptr<events::Spawn> t) {
    if (t != nullptr) {
      std::unique_lock<std::mutex> lck(actorsMtx);
      actors.insert(t->Actor());
    }
  })(std::dynamic_pointer_cast<events::Spawn>(e));

  // Remove an actor
  ([=](std::shared_ptr<events::Destroy> t) {
    if (t != nullptr) {
      std::unique_lock<std::mutex> lck(actorsMtx);
      actors.erase(t->Actor());
    }
  })(std::dynamic_pointer_cast<events::Destroy>(e));
}

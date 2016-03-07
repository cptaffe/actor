// Copyright 2016 Connor Taffe

#include "src/spool.h"

Spool *Spool::instance = nullptr;

void Spool::Handle(std::shared_ptr<Event> e) {
  std::vector<std::pair<std::shared_ptr<Actor>, std::shared_ptr<Event>>> v;
  for (auto a : ([&]() {
         std::unique_lock<std::mutex> lck(actorsMtx);
         std::vector<std::shared_ptr<Actor>> vec;
         for (auto a : actors) {
           vec.push_back(a.second);
         }
         return vec;
       })()) {
    v.push_back({a, e});
  }
  handles.Put(v);

  // Also handle the event
  ([=](std::shared_ptr<interfaces::Terminal> t) {
    if (t != nullptr) {
      handles.Kill();
    }
  })(std::dynamic_pointer_cast<interfaces::Terminal>(e));
}

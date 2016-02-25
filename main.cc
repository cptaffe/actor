
#include <chrono>
#include <iostream>
#include <map>
#include <string>
#include <thread>

#include "actor.h"
#include "events.h"
#include "interfaces.h"

Spool *Spool::instance = new Spool;

int main() {
  auto s = Spool::Instance();
  s->Register("king", new actors::King());
  s->Register("innkeeper", new actors::Npc());
  s->Register("speech", new actors::Sayer());
  auto t = new std::thread([&] {
    for (auto i = 0; i < 10; i++) {
      s->Handle(new events::Say(nullptr, "hey"));
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  });
  s->Run(8);
  t->join();
  // Terminates the consumers
  s->Handle(new events::Terminate("die!"));
  s->Wait();
  delete t;
}


#include <chrono>
#include <iostream>
#include <map>
#include <string>
#include <thread>

#include "actor.h"
#include "events.h"
#include "interfaces.h"

events::EventSpool *events::EventSpool::instance = new events::EventSpool;

int main() {
  auto s = events::EventSpool::Instance();
  s->RegisterActor("king", new actors::King());
  s->RegisterActor("innkeeper", new actors::Npc());
  s->RegisterActor("speech", new actors::Sayer());
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

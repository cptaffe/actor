
#include <iostream>
#include <map>
#include <string>

#include "actor.h"
#include "events.h"
#include "interfaces.h"

events::EventSpool *events::EventSpool::instance = new events::EventSpool;

int main() {
  events::EventSpool::Instance()->RegisterActor(new actors::King());
  events::EventSpool::Instance()->RegisterActor(new actors::Npc());
  events::EventSpool::Instance()->RegisterActor(new actors::Sayer());
  events::EventSpool::Instance()->Handle(new events::Say("hey"));
  events::EventSpool::Instance()->Wait();
}

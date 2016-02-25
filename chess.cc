
// Chess implemented as Actors
#include "base.h"
#include "events.h"
#include "spool.h"
#include "util.h"
#include <array>
#include <atomic>
#include <random>
#include <vector>

namespace chess {

using Board = std::array<std::array<char, 8>, 8>;
using Pos = std::pair<int, int>;

namespace events {
namespace interfaces {

// Event indicates a move
class Move {
public:
  virtual ~Move() {}
  virtual Pos To() = 0;
  virtual Pos From() = 0;
};

class Whoable {
public:
  virtual ~Whoable() {}
  virtual Actor *Who() = 0;
};

} // namespace interfaces

class TryMove : public Event, public interfaces::Whoable {
public:
  TryMove(Actor *a, Pos to) : actor(a), toPosition(to) {}
  virtual Actor *Who() override { return actor; }
  Pos To() { return toPosition; }
  virtual std::string Description() override { return "Attempt to do a move"; }

private:
  Actor *actor;
  Pos toPosition;
};

class Move : public Event, public interfaces::Move {
public:
  Move(Pos f, Pos t) : from(f), to(t) {}
  virtual Pos To() override { return to; }
  virtual Pos From() override { return from; }
  virtual std::string Description() override { return "Moving a piece"; }

private:
  Pos from, to;
};

class HasPos : public Event, public interfaces::Whoable {
public:
  HasPos(Actor *a, Pos p) : actor(a), position(p) {}
  Pos Position() { return position; }
  virtual Actor *Who() override { return actor; }
  virtual std::string Description() override {
    return "This position is taken";
  }

private:
  Actor *actor;
  Pos position;
};

class Taken : public Event, public interfaces::Whoable {
public:
  Taken(Actor *a) : actor(a) {}
  virtual Actor *Who() override { return actor; }
  virtual std::string Description() override {
    return "A piece has been taken";
  }

private:
  Actor *actor;
};

} // namespace events

namespace actors {
namespace interfaces {}

class Rook : public Actor, public Renderable {
public:
  Rook(Pos p) : position(p) {}
  virtual void Render() override {
    // TODO: implement
  }
  virtual void Handle(Event *e) override {
    ([=](events::TryMove *t) {
      if (t != nullptr) {
        if (this == t->Who()) {
          // Attempt to do the move,
          // if successful issue a Move event
          auto tp = t->To();
          // One must be true, and only one (cannot move to same position)
          if (CheckMove(position, tp) && !taken) {
            // Can only move along the axes.
            Spool::Instance()->Handle(new events::Move(position, tp));
            position = tp;
          } else {
            // Move is not possible
          }
        } else if (t->To() == position) {
          // Has position
          Spool::Instance()->Handle(new events::HasPos(this, position));
        }
      }
    })(dynamic_cast<events::TryMove *>(e));
    ([=](events::Move *m) {
      if (m != nullptr) {
        // Unregister from future events
        Spool::Instance()->Unregister(this);
        taken = true;
        Spool::Instance()->Handle(new events::Taken(this));
      }
    })(dynamic_cast<events::Move *>(e));
  }

  static bool CheckMove(Pos o, Pos t) {
    return t.first == o.first != t.second == o.second;
  }

private:
  bool taken = false;
  Pos position;
};

class Bishop : public Actor, public Renderable {
public:
  Bishop(Pos p) : position(p) {}
  virtual void Render() override {
    // TODO: implement
  }
  virtual void Handle(Event *e) override {
    ([=](events::TryMove *t) {
      if (t != nullptr) {
        if (this == t->Who()) {
          // Attempt to do the move,
          // if successful issue a Move event
          auto tp = t->To();
          if (CheckMove(position, tp) && !taken) {
            // Can only move along the diagonals.
            Spool::Instance()->Handle(new events::Move(position, tp));
            position = tp;
          } else {
            // Move is not possible
          }
        } else if (t->To() == position) {
          // Has position
          Spool::Instance()->Handle(new events::HasPos(this, position));
        }
      }
    })(dynamic_cast<events::TryMove *>(e));
    ([=](events::Move *m) {
      if (m != nullptr) {
        // Unregister from future events
        Spool::Instance()->Unregister(this);
        taken = true;
        Spool::Instance()->Handle(new events::Taken(this));
      }
    })(dynamic_cast<events::Move *>(e));
  }

  static bool CheckMove(Pos o, Pos t) {
    return o != t && t.first - o.first == t.second - o.second;
  }

private:
  bool taken = false;
  Pos position;
};

} // namespace actors

class GameBuilder {
public:
  std::vector<std::pair<std::string, Actor *>> Build() {
    std::vector<std::pair<std::string, Actor *>> v;
    for (auto b : builders) {
      for (auto a : b()) {
        v.push_back(a);
      }
    }
    return v;
  }

private:
  std::vector<std::function<std::vector<std::pair<std::string, Actor *>>()>>
      builders = {[] {
        return std::vector<std::pair<std::string, Actor *>>({
            {"white-r1", new actors::Rook({0, 0})},
            {"white-r2", new actors::Rook({0, 7})},
            {"black-r1", new actors::Rook({7, 0})},
            {"black-r2", new actors::Rook({7, 7})},
            {"white-b1", new actors::Bishop({0, 2})},
            {"white-b2", new actors::Bishop({0, 5})},
            {"black-b1", new actors::Bishop({7, 2})},
            {"black-b2", new actors::Bishop({7, 5})},
        });
      }};
};

} // namespace chess

Spool *Spool::instance = new Spool;

class Logger : public Actor {
public:
  virtual void Handle(Event *e) override {
    ([=](chess::events::Move *m) {
      if (m != nullptr) {
        std::cout << "MOVE! " << e->Description() << std::endl;
      }
    })(dynamic_cast<chess::events::Move *>(e));
    std::cout << e->Description() << std::endl;
  }
};

class Renderer {
public:
  Renderer(std::vector<Renderable *> r) : renderables(r) {}

  void Render() {
    for (auto r : renderables) {
      r->Render();
    }
  }

private:
  std::vector<Renderable *> renderables;
};

class RandomGamer : public Actor {
public:
  RandomGamer(std::vector<std::string> h)
      : handles(h), thread([=] {
          for (;;) {
            Move();
          }
        }) {}

  virtual void Handle(Event *e) override {
    ([=](interfaces::Terminal *t) {})(dynamic_cast<interfaces::Terminal *>(e));
  }

  void Wait() { thread.join(); }

private:
  std::vector<std::string> handles;
  std::mt19937_64 random;
  std::thread thread;

  void Move() {
    Spool::Instance()->Handle(new chess::events::TryMove(
        Spool::Instance()->ActorById(std::bind(
            [=](int i) { return handles[i]; },
            std::bind(std::uniform_int_distribution<int>(0, handles.size()),
                      random))()),
        ([=] {
          auto rc = std::bind(std::uniform_int_distribution<int>(0, 8), random);
          return chess::Pos({rc(), rc()});
        })()));
  }
};

int main() {
  auto s = Spool::Instance();
  auto actors = chess::GameBuilder().Build();
  for (auto a : actors) {
    s->Register(a.first, a.second);
  }
  s->Register("logger", new Logger());
  s->Register("player", new RandomGamer(([=] {
                std::vector<std::string> h;
                for (auto a : actors) {
                  h.push_back(a.first);
                }
                return h;
              })()));
  s->Run(8);

  // Render once
  Renderer renderer(([=] {
    std::vector<Renderable *> v;
    for (auto a : actors) {
      ([&](Renderable *r) {
        if (r != nullptr) {
          v.push_back(r);
        }
      })(dynamic_cast<Renderable *>(a.second));
    }
    return v;
  })());
  renderer.Render();

  // Terminates the consumers
  static_cast<RandomGamer *>(s->ActorById("player"))->Wait();
  s->Handle(new events::Terminate("die!"));
  s->Wait();
}

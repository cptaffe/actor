// Copyright (c) 2016 Connor Taffe

#ifndef SRC_RENDERER_EVENT_EVENT_H_
#define SRC_RENDERER_EVENT_EVENT_H_

#include <memory>
#include <string>
#include <vector>

#include "src/base.h"
#include "src/renderer/renderer.h"

namespace event {

class Spawn : public Event {
 public:
  Spawn(std::shared_ptr<renderer::Rasterizable> rast,
        std::vector<std::shared_ptr<renderer::Renderable>> rend)
      : rasterizable{rast}, renderers{rend} {}
  std::string Description() override {
    return "event::Spawn: A rasterizable was spawned";
  }
  std::shared_ptr<renderer::Rasterizable> Display() { return rasterizable; }
  std::vector<std::shared_ptr<renderer::Renderable>> Model() {
    return renderers;
  }

 private:
  std::shared_ptr<renderer::Rasterizable> rasterizable;
  std::vector<std::shared_ptr<renderer::Renderable>> renderers;
};

}  // namespace event

#endif  // SRC_RENDERER_EVENT_EVENT_H_

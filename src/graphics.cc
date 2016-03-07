// Copyright 2016 Connor Taffe

#include "src/graphics.h"
#include "src/renderer/event/event.h"

namespace renderables {

Scale::Scale(glm::vec3 s) : scale{s} {}

std::vector<glm::mat4> Scale::Render() const { return {glm::scale(scale)}; }

Translate::Translate(glm::vec3 t) : translation{glm::translate(t)} {}

std::vector<glm::mat4> Translate::Render() const { return {translation}; }

Rotate::Rotate(double a, glm::vec3 v) : angle{a}, vec{v} {}

std::vector<glm::mat4> Rotate::Render() const {
  return {glm::rotate(static_cast<float>(angle), vec)};
}

Float::Float(double rad, std::chrono::duration<double> d)
    : radius{rad}, duration{d} {}

std::vector<glm::mat4> Float::Render() const {
  return Translate{
      glm::vec3{0, radius * glm::cos(renderer::Timer::Instance()->Since() /
                                     duration * 2 * glm::pi<double>()),
                0}}
      .Render();
}

Spin::Spin(std::chrono::duration<double> d) : duration{d} {}

std::vector<glm::mat4> Spin::Render() const {
  return Rotate{
      renderer::Timer::Instance()->Since() / duration * 2 * glm::pi<double>(),
      glm::vec3{0, 1, 0}}
      .Render();
}

MatRenderable::MatRenderable(glm::mat4 m) : matrix{m} {}

}  // namespace renderables

Fps::Fps() : ticks{std::chrono::high_resolution_clock::now()}, diff{0} {}

void Fps::Update() {
  auto now = std::chrono::high_resolution_clock::now();
  diff = now - ticks;
  ticks = now;
}

double Fps::Frame() const {
  return 1.0 /
         std::chrono::duration_cast<std::chrono::duration<double>>(diff)
             .count();
}

std::ostream &operator<<(std::ostream &o, const Fps &fps) {
  return o << fps.Frame();
}

int main(int argc, const char *argv[]) {
  if (argc != 3) {
    throw std::runtime_error("./graphics cubes windows");
  }
  int cubes, windows;
  std::stringstream(argv[1]) >> cubes;
  std::stringstream(argv[2]) >> windows;

  std::cout << "spawning " << windows << " windows with " << cubes << " cubes"
            << std::endl;

  auto s = Spool::Instance();
  s->Register("speech", std::shared_ptr<Actor>(new actors::Sayer()));
  s->Run();  // run spool

  auto renderers = std::vector<std::shared_ptr<renderer::Renderer>>();
  for (auto i = 0; i < windows; i++) {
    const auto renderer =
        renderer::Builder()
            .View(std::shared_ptr<renderer::Renderable>(
                new renderables::MatRenderable(
                    glm::lookAt(glm::vec3(4, 3, 3), glm::vec3(0, 0, 0),
                                glm::vec3(0, 1, 0)))))
            .Projection([](size_t w, size_t h) {
              return std::shared_ptr<renderer::Renderable>(
                  new renderables::MatRenderable(glm::perspective(
                      0.5 * glm::pi<double>(),
                      static_cast<double>(w) /
                          static_cast<double>(h),  // should be window ratio
                      0.1,
                      100.0)));
            })
            .Build();
    renderers.push_back(renderer);
    s->Register(
        static_cast<std::stringstream &>(std::stringstream() << "renderer" << i)
            .str(),
        std::shared_ptr<Actor>{renderer});
  }

  std::mt19937_64 random;
  auto rand = std::bind(std::uniform_real_distribution<double>(-1, 1), random);
  for (auto &renderer : renderers) {
    for (auto i = 0; i < cubes; i++) {
      s->Handle(std::unique_ptr<Event>(new event::Spawn(
          renderer->ShapeFactory()->Cube(),
          std::vector<std::shared_ptr<renderer::Renderable>>(
              {std::shared_ptr<renderer::Renderable>(
                   new renderables::Translate({rand(), rand(), rand()})),
               std::shared_ptr<renderer::Renderable>(
                   new renderables::Scale({0.25, 0.25, 0.25})),
               std::shared_ptr<renderer::Renderable>(new renderables::Float(
                   0.25, std::chrono::milliseconds(
                             std::uniform_int_distribution<uint64_t>(
                                 1000, 5000)(random)))),
               std::shared_ptr<renderer::Renderable>(
                   new renderables::Spin(std::chrono::milliseconds(
                       std::uniform_int_distribution<uint64_t>(
                           1000, 6000)(random))))}))));
    }
  }

  s->Wait();
}

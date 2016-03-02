
#include <chrono>
#include <functional>
#include <random>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "renderers/builder.h"
#include "renderers/renderer.h"

class Scale : public renderer::Renderable {
public:
  Scale(glm::vec3 s) : scale(s) {}
  virtual std::vector<glm::mat4> Render() override {
    return {glm::scale(scale)};
  }

private:
  glm::vec3 scale;
};

class Translate : public renderer::Renderable {
public:
  Translate(glm::vec3 t) : translation(glm::translate(t)) {}
  virtual std::vector<glm::mat4> Render() override { return {translation}; }

private:
  glm::mat4 translation;
};

class Rotate : public renderer::Renderable {
public:
  Rotate(double a, glm::vec3 v) : angle(a), vec(v) {}
  virtual std::vector<glm::mat4> Render() override {
    return {glm::rotate(static_cast<float>(angle), vec)};
  }

private:
  double angle;
  glm::vec3 vec;
};

class Float : public renderer::Renderable {

public:
  Float(double rad, std::chrono::duration<double> d)
      : radius(rad), duration(d) {}
  virtual std::vector<glm::mat4> Render() override {
    return Translate(glm::vec3(0, radius * glm::cos(timer.Since() / duration *
                                                    2 * glm::pi<double>()),
                               0))
        .Render();
  }

private:
  double radius;
  std::chrono::duration<double> duration;
  renderer::Timer timer;
};

class Spin : public renderer::Renderable {

public:
  Spin(std::chrono::duration<double> d) : duration(d) {}
  virtual std::vector<glm::mat4> Render() {
    return Rotate(timer.Since() / duration * 2 * glm::pi<double>(),
                  glm::vec3(0, 1, 0))
        .Render();
  }

private:
  std::chrono::duration<double> duration;
  renderer::Timer timer;
};

int main() {
  auto cubes = 10;
  auto renderer =
      renderer::Builder()
          .View(glm::lookAt(glm::vec3(4, 3, 3), glm::vec3(0, 0, 0),
                            glm::vec3(0, 1, 0)))
          .Projection(glm::perspective(0.5 * glm::pi<double>(),
                                       1.0, // should be window ratio
                                       0.1, 100.0))
          .Build();
  std::mt19937_64 random;
  auto rand = std::bind(std::uniform_real_distribution<double>(-1, 1), random);
  for (auto i = 0; i < cubes; i++) {
    renderer->AddRenderable(
        renderer->ShapeFactory()->Cube(),
        std::vector<renderer::Renderable *>(
            {new Translate({rand(), rand(), rand()}),
             new Scale({0.25, 0.25, 0.25}),
             new Float(0.25, std::chrono::milliseconds(
                                 std::uniform_int_distribution<long>(
                                     1000, 5000)(random))),
             new Spin(std::chrono::milliseconds(
                 std::uniform_int_distribution<long>(1000, 6000)(random)))}));
  }

  renderer::Timer::Start();
  auto t = std::chrono::high_resolution_clock::now();
  for (;;) {
    renderer::Timer::Stop();
    try {
      renderer->Render();
    } catch (const std::exception &e) {
      std::cerr << e.what() << std::endl;
      break;
    }
    auto nt = std::chrono::high_resolution_clock::now();
    std::cout << 1 /
                     std::chrono::duration_cast<std::chrono::duration<float>>(
                         nt - t)
                         .count()
              << "\r" << std::flush;
    t = nt;
  }
}


#include <chrono>
#include <functional>
#include <random>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "renderers/gl/gl.h"
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

class Triangle : public renderer::Renderable {
public:
  Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c)
      : vertices(glm::vec4(a, 1), glm::vec4(b, 1), glm::vec4(c, 1),
                 glm::vec4(1)) {}
  virtual std::vector<glm::mat4> Render() override { return {vertices}; }

private:
  glm::mat4 vertices;
};

class EquilateralTriangle : public renderer::Renderable {
public:
  EquilateralTriangle()
      : triangle(glm::vec3(1, -1, 0), glm::vec3(-1, -1, 0),
                 glm::vec3(0, 1, 0)) {}
  virtual std::vector<glm::mat4> Render() override { return triangle.Render(); }

private:
  Triangle triangle;
};

class Square : public renderer::Renderable {
public:
  Square()
      : triangles(([=] {
          auto triangles = std::vector<Triangle>(
              {{glm::vec3(1, -1, 0), glm::vec3(-1, 1, 0), glm::vec3(1, 1, 0)},
               {glm::vec3(1, -1, 0), glm::vec3(-1, 1, 0),
                glm::vec3(-1, -1, 0)}});
          std::vector<glm::mat4> v;
          for (auto t : triangles) {
            for (auto m : t.Render()) {
              v.push_back(m);
            }
          }
          return v;
        })()) {}
  virtual std::vector<glm::mat4> Render() override { return triangles; }

private:
  std::vector<glm::mat4> triangles;
};

class Cube : public renderer::Renderable {

public:
  Cube()
      : sides(([=] {
          auto s0 = Square().Apply(Translate({0, 0, 1}).Render());
          auto s1 = Square().Apply(Translate({0, 0, 1}).Apply(
              Rotate(0.5 * glm::pi<double>(), glm::vec3(0, 1, 0)).Render()));

          auto s2 = Square().Apply(Translate({0, 0, 1}).Apply(
              Rotate(-0.5 * glm::pi<double>(), glm::vec3(0, 1, 0)).Render()));
          auto s3 = Square().Apply(Translate({0, 0, -1}).Render());
          auto s4 = Square().Apply(Translate({0, 0, 1}).Apply(
              Rotate(0.5 * glm::pi<double>(), glm::vec3(1, 0, 0)).Render()));
          auto s5 = Square().Apply(Translate({0, 0, 1}).Apply(
              Rotate(-0.5 * glm::pi<double>(), glm::vec3(1, 0, 0)).Render()));
          s0.insert(std::end(s0), std::begin(s1), std::end(s1));
          s0.insert(std::end(s0), std::begin(s2), std::end(s2));
          s0.insert(std::end(s0), std::begin(s3), std::end(s3));
          s0.insert(std::end(s0), std::begin(s4), std::end(s4));
          s0.insert(std::end(s0), std::begin(s5), std::end(s5));
          return s0;
        })()) {}
  virtual std::vector<glm::mat4> Render() override { return sides; }

private:
  std::vector<glm::mat4> sides;
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
  gl::Renderer renderer(
      ([=] {
        std::mt19937_64 random;
        auto rand =
            std::bind(std::uniform_real_distribution<double>(-1, 1), random);
        std::vector<std::vector<renderer::Renderable *>> vec;
        auto cube = [&] {
          return std::vector<renderer::Renderable *>(
              {new Translate({rand(), rand(), rand()}),
               new Scale({0.25, 0.25, 0.25}),
               new Float(0.25, std::chrono::milliseconds(
                                   std::uniform_int_distribution<long>(
                                       1000, 5000)(random))),
               new Spin(std::chrono::milliseconds(
                   std::uniform_int_distribution<long>(1000, 6000)(random)))});
        };
        for (auto i = 0; i < cubes; i++) {
          vec.push_back(cube());
        }
        return vec;
      })(),
      ([=] {
        std::vector<renderer::Renderable *> v;
        for (auto i = 0; i < cubes; i++) {
          v.push_back(new Cube());
        }
        return v;
      })(),
      glm::lookAt(glm::vec3(4, 3, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)),
      glm::perspective(0.5 * glm::pi<double>(),
                       1.0, // should be window ratio
                       0.1, 100.0));

  renderer::Timer::Start();
  auto t = std::chrono::high_resolution_clock::now();
  for (;;) {
    renderer::Timer::Stop();
    try {
      renderer.Render();
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

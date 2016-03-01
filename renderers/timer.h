
#ifndef B_RENDERERS_TIMER_H_
#define B_RENDERERS_TIMER_H_

#include <chrono>

namespace renderer {

class Timer {
public:
  Timer() {}

  std::chrono::duration<double> Since() { return stop - last; }

  static void Start() { last = std::chrono::high_resolution_clock::now(); }
  static void Stop() { stop = std::chrono::high_resolution_clock::now(); }

private:
  static std::chrono::time_point<std::chrono::high_resolution_clock> stop, last;
};

} // namespace renderer

#endif // B_RENDERERS_TIMER_H_

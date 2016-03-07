// Copyright 2016 Connor Taffe

#ifndef SRC_RENDERER_TIMER_H_
#define SRC_RENDERER_TIMER_H_

#include <chrono>

namespace renderer {

class Timer {
 public:
  static Timer *Instance() {
    if (instance == nullptr) {
      instance = new Timer();
    }
    return instance;
  }
  std::chrono::duration<double> Since() { return stop - last; }
  void Stop() { stop = std::chrono::high_resolution_clock::now(); }

  static void Start() { last = std::chrono::high_resolution_clock::now(); }

 private:
  Timer() {}
  Timer(const Timer &) = delete;
  // Time-pausing thread-local
  static thread_local Timer *instance;
  // Time-beginning thread-global
  static std::chrono::time_point<std::chrono::high_resolution_clock> last;
  std::chrono::time_point<std::chrono::high_resolution_clock> stop;
};

}  // namespace renderer

#endif  // SRC_RENDERER_TIMER_H_

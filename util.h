
#ifndef B_UTIL_H_
#define B_UTIL_H_

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

template <typename T> class ConsumerQueue {
public:
  ConsumerQueue(std::function<void(T)> c) : consumer(c) {}

  void Put(T t) {
    std::unique_lock<std::mutex> lock(mutex);

    // Ignore events if terminated
    if (alive) {
      queue.push(t);
      condition.notify_one();
    }
  }

  void Kill() {
    std::unique_lock<std::mutex> lock(mutex);
    alive = false;          // end queue
    condition.notify_all(); // tell all consumers
  }

  void Run(int t) {
    for (int i = 0; i < t; i++) {
      threads.push_back(new std::thread([=] { Consume(); }));
    }
  }

  void Wait() {
    // Join worker threads
    for (auto t : threads) {
      t->join();
    }
  }

private:
  std::function<void(T)> consumer;
  std::vector<std::thread *> threads;
  std::mutex mutex;
  std::condition_variable condition;
  bool alive = true; // set to false once terminated
  std::queue<T> queue;

  void Consume() {
    for (;;) {
      std::unique_lock<std::mutex> lock(mutex);
      condition.wait(lock, [&] { return !queue.empty() || !alive; });
      if (queue.empty()) {
        // Dead and no events left to process
        return;
      }
      auto t = queue.front();
      queue.pop();
      lock.unlock(); // unlocks the lock
      consumer(t);
    }
  }
};

#endif // B_UTIL_H_

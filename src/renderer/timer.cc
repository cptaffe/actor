// Copyright 2016 Connor Taffe

#include "src/renderer/timer.h"

thread_local renderer::Timer *renderer::Timer::instance = nullptr;
std::chrono::time_point<std::chrono::high_resolution_clock>
    renderer::Timer::last;

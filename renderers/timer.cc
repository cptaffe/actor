
#include "timer.h"

std::chrono::time_point<std::chrono::high_resolution_clock>
    renderer::Timer::stop;
std::chrono::time_point<std::chrono::high_resolution_clock>
    renderer::Timer::last;

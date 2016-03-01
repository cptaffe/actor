# actor
Actor model thing

To run on linux use the following:
```sh
clang++ graphics.cc renderers/timer.cc renderers/gl/buffer.cc renderers/gl/gl.cc -o graphics.out --std=c++1z -g -Wall -lglfw -lGLEW -lGLU -lGL -lpthread
./graphics.out
```

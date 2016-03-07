# actor
Actor model thing

To run on linux use the following:
```sh
clang++ src/renderer/renderers/gl/buffer.cc src/renderer/renderers/gl/renderer.cc src/renderer/renderers/gl/shader.cc src/renderer/renderers/gl/window.cc src/renderer/renderers/gl/buffer.cc src/renderer/renderers/gl/shapes.cc src/renderer/renderer.cc src/renderer/timer.cc src/actor.cc src/base.cc src/events.cc src/graphics.cc src/interfaces.cc src/spool.cc -o graphics.out --std=c++1z -g -Wall -lglfw -lGLEW -lGLU -lGL -lpthread -I.
```

# FemtoLib

A library for games on resource-constrained systems.

This library was written with the following goals in mind:
- It should be approachable. One should be able to look at a simple example and have a good idea of how to get started.
- It should be light-weight. Don't pay for features that aren't used.
- It should be flexible. It should be possible to create many different kinds of games, even if that means overloading parts of the library with custom functionality.
- It should be platform-independent.

That said, this is the smallest possible FemtoLib program:
```cpp
void update(){}
```

Currently, this compiles into a 17kb binary on the Pokitto. Most of that space is due to the fact that games are responsible for calling and updating the loader on the Pokitto, and omitting that would bring problems to end-users.

Instead of a `main()` function, FemtoLib games need to implement an `update()` function that gets called every frame. Moving the game loop into the library allows greater portability to platforms such as emscripten.


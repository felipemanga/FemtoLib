# FemtoLib

## Intro

A library for games on resource-constrained systems.

This library was written with the following goals in mind:
- It should be approachable. One should be able to look at a simple example and have a good idea of how to get started.
- It should be light-weight. Don't pay for features that aren't used.
- It should be flexible. It should be possible to create many different kinds of games, even if that means overloading parts of the library with custom functionality.
- It should be platform-independent.

This is the list of currently supported platforms, with more coming soon:
- Windows
- Linux
- MacOS
- Web Browsers (via WebAssembly / Emscripten)
- Pokitto
- Nintendo 3DS

## Getting Started

Now for some code. This is the smallest/simplest possible FemtoLib program:
```cpp
void update(){}
```

Instead of a `main()` function, FemtoLib games need to implement an `update()` function that gets called every frame. Moving the game loop into the library allows greater portability to platforms such as emscripten.

Actually drawing something onto the screen can be done like this:
```cpp
    #include <Femto>

    // allow calling functions without having to use a namespace (Graphics::clear())
    using namespace Graphics;

    void update() {
        // clear the screen
        clear();

        // draw a 40x40 square on coordinate 10, 20.
        fillRect(Point2D{10.0f, 20.0f}, Size2D{40, 40}, colorFromRGB(0x88AA33));
    }
```

You can also fill a rectangle like this:

```cpp
    primaryColor = colorFromRGB(0x88AA33);
    fillRect({10, 20}, Size2D{40, 40});
```

Note that `Point2D` coordinates are `f32`. These are either fixed- or floating-point, depending on whether the hardware supports floating-point numbers or not. Also, coordinates specified as `Point2D` are camera-relative.

It is also possible to draw a filled rectangle by specifying absolute coordinates for the top-left and bottom-right corners like this:

```cpp
    camera.set(5, 5); // offset all camera-relative coordinates by {5, 5} pixels.
    fillRect({15, 25}, Point2D{55, 65});
```

You can also use int (`s32`) coordinates, but these aren't camera-relative:

```cpp
    fillRect(10, 20, 40, 40);
```

Other than filled rectangles, it is also possible to draw lines:

```cpp
    line({10.0f, 20.0f}, {40.0f, 40.0f}, colorFromRGB(0));
```

And text:
```cpp
    setCursor({10.0f, 10.0f}); // or setCursor(10, 10) if not camera-relative
    print("Hello World! ", random(), " is your lucky number today!\n");
```

Of course, it is also possible to draw bitmaps:
```cpp
    #include <Femto>
    using namespace Graphics;

    BITMAP(button, 16, 16,
       1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
       1, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1,
       1, 2, 8, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 8, 2, 1,
       1, 2, 2, 8, 2, 2, 2, 2, 2, 2, 2, 2, 8, 2, 2, 1,
       1, 2, 2, 2, 8, 2, 2, 2, 2, 2, 2, 8, 2, 2, 2, 1,
       1, 2, 2, 2, 2, 8, 2, 2, 2, 2, 8, 2, 2, 2, 2, 1,
       1, 2, 2, 2, 2, 2, 8, 2, 2, 8, 2, 2, 2, 2, 2, 1,
       1, 2, 2, 2, 2, 2, 2, 8, 8, 2, 2, 2, 2, 2, 2, 1,
       1, 2, 2, 2, 2, 2, 2, 8, 8, 2, 2, 2, 2, 2, 2, 1,
       1, 2, 2, 2, 2, 2, 8, 2, 2, 8, 2, 2, 2, 2, 2, 1,
       1, 2, 2, 2, 2, 8, 2, 2, 2, 2, 8, 2, 2, 2, 2, 1,
       1, 2, 2, 2, 8, 2, 2, 2, 2, 2, 2, 8, 2, 2, 2, 1,
       1, 2, 2, 8, 2, 2, 2, 2, 2, 2, 2, 2, 8, 2, 2, 1,
       1, 2, 8, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 8, 2, 1,
       1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1,
       1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
    );

    void update(){
         draw(button, {10.0f, 10.0f});
    }
```

The BITMAP macro ensures that data is stored in the most compact form possible. It will automatically choose between 1, 2, 4 or 8 bits per pixel. It is also important for supporting future platforms where such data might need to be treated differently.

It is possible to force a bitmap to be stored with a given encoding, if necessary:

```cpp
    BITMAP8BPP(name, width, height, ...); // Creates a 256-color bitmap
    BITMAP4BPP(name, width, height, ...); // Creates a 16-color bitmap
    BITMAP2BPP(name, width, height, ...); // Creates a 4-color bitmap
    BITMAP1BPP(name, width, height, ...); // Creates a 2-color bitmap
```

BITMAP also supports multiple frames for tiles or animated sprites:
```cpp
    BITMAP(ball, 3, 3,
        // frame 1
         0, 20,  0,
        20, 30, 20,
         0, 20,  0,

        // frame 2
         0, 22,  0,
        22, 37, 22,
         0, 22,  0,

        // frame 3
         0, 28,  0,
        28, 42, 28,
         0, 28,  0
    );

    draw(ball[frame++%3], 10, 10); // cycle between frames 0, 1 and 2.
```

By default, color index 0 will not be drawn. If sprites do not have transparent areas,
you can disable color keying like this:
```cpp
    draw<false>(ball, 10, 10); // transparent = false
```

It is also possible to draw sprites with alpha blending:
```cpp
    draw(ball, 10, 10, 0.5); // 50% opacity
```

And you can have sprites that use different palettes:
```cpp
    Graphics::palette = Graphics::generalPalette;
    draw(ball, 10, 10); // draw ball with the built-in general-purpose palette

    Graphics::palette = customPalette;
    draw(ball, 10, 10); // draw ball with another palette
```

The "regular" image format is also supported, but you will have to manually specify the BPP when drawing:

```cpp
    constexpr u8 image[] = {
    3, 3,
    0, 128, 0,
    128, 255, 0,
    0, 128, 0
    };

    draw<true, 8>(image, 10, 10); // note the "8"
```

## The Rendering System

FemtoLib's graphics system is capable of much more than this, due to its use of _layers_.
By creating a _renderer_ with the right layers, you can fine-tune the system to do what you need. It is also possible to customize the Audio system, LibAudio.

For this, you need to provide your own `init` function.
This is what the default init looks like:

```cpp
void init() {
     Graphics::palette = Graphics::generalPalette;
     Graphics::init();
     Audio::init();
     setMaxFPS(0);
}
```

It is important that your `init` function also does these things, but not necessarily in the same way. Let's go over each line.

### `Graphics::palette = Graphics::generalPalette;`

This sets the color palette to be used by bitmaps. Trying to draw an indexed bitmap without a palette will likely result in a crash. Have a look at the `generalPalette` in [general.hpp](general.hpp) to see how to make your own palettes.

### `Graphics::init();`

All the `Graphics::init()` function does is this:
```cpp
    inline void Graphics::init(){
        static Graphics::Renderer<
            Graphics::layer::SolidColor<bgColor>,
            Graphics::layer::DrawList<spriteCount, font>
            > defaultGfx;
    }
```

This is a very simple layer stack: First it fills the screen with a SolidColor background, then it executes a list of drawing commands. `line`, `draw`, `fillRect`, and even `print`, each one produces one or more commands that are added to the DrawList. After your update function returns, the commands are applied and the screen is updated. The `clear()` command empties the DrawList for the next frame.

### `Audio::init();`

This is also a simple function. All it does is initialize LibAudio with 4 channels at 8khz.
```cpp
inline void init(){
    static Audio::Sink<4, 8000> defaultAudio;
}
```

### `setMaxFPS(0);`

For development, it is useful to not have a frame-rate limiter, so setting the maximum to zero disables the limiter entirely. However, in the end you'll want to limit the maximum frame-rate so that the game behaves as expected on all systems. Many devices can't refresh the screen faster than 30 FPS, so setting that to be the maximum is a good idea.

### A Custom Init

Now let's make our own init function.

```cpp
    // We can make our renderer as a global variable to keep things simple
    // If it is a global variable, it doesn't need to be static.
    // Don't define renderer in a header file.
    Graphics::Renderer<

        // I'll use a Tilemap as a background layer.
        // Each tile will be 16 pixels wide, 16 pixels tall.
        Graphics::layer::Tilemap<16, 16>,

        // Render text on top using the MonoText layer.
        Graphics::layer::MonoText<fontTiny>

    > renderer;

    // Same thing for Audio. Just two channels, but running at 16KHz.
    Audio::Sink<2, 16000> fancyAudio;

    void init() {
        // now all that's left to do is set a palette and the max FPS
        Graphics::palette = Graphics::generalPalette; // still using the built-in palette
        setMaxFPS(30);
    }
```

Note that there isn't a DrawList layer. That means that commands like `line` or `draw` won't work. `print`, however, will be handled by the MonoText layer, which is specialized at rendering screenfulls of mono-space text.

As for the Tilemap layer, for it to do its thing, it needs a tileset and a tilemap.
Those can be specified like this:

```cpp
void init() { 
   ...
    Graphics::setTileset(tiles);
    Graphics::setTilemap(map);
}
```

Generally, both `tiles` and `map` are BITMAPs.

# Fractal Tree Creator

An app using OpenGL that lets you configure your own 2D fractal tree.

## Screenshots

![Savannah Tree](docs/screenshots/savannah_tree.png)

![Christmas Tree](docs/screenshots/christmas_tree.png)

![Basic Tree](docs/screenshots/basic_tree.png)

## Installation

Download the [Latest Release](https://github.com/Ev01/FractalTree/releases).
Fractal Tree Creator requires your system to support at least OpenGL 3.3.

## Features / Controls

- Saving and loading tree configurations.
- Middle click and drag to pan.
- Scroll to zoom

## Compile from source

To compile from source, you must have SDL3 installed on your system.
You can get it [here](https://github.com/libsdl-org/SDL) 
([Installation instructions for SDL](https://github.com/libsdl-org/SDL/blob/main/docs/README-cmake.md)).

Then to download the source code and compile:
```
git clone https://github.com/Ev01/FractalTreeCreator.git
cd FractalTreeCreator
cmake -S . -B build
cmake --build build
```

And to run the program:
```
./build/fractal_tree
```

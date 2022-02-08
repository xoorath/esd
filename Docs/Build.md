# Electrostatic Discharge

## Building

[Home](../README.md) / [Docs](./Readme.md) / *Building*

#### Windows

Download and install [CMake](https://cmake.org/) and [Visual Studio 2019](https://visualstudio.microsoft.com/vs/older-downloads/).

The following commands will generate a visual studio solution at `/Build/esd.sln` and build it to `/Build/Release/esd.exe`.

```
cmake -S . -B Build -A x64 -G "Visual Studio 16 2019"
cmake --build Build --config Release
```

*Yes: Other toolchains and generators for Windows are going to work fine if you know what you're doing.*

#### Linux & MacOS

Download and install [CMake](https://cmake.org/) as well as [Ninja](https://ninja-build.org/).

On linux install build-essential with `sudo apt install build-essential`.

On MacOS install gcc using brew with `brew install gcc`.

The following commands will build esd using the Ninja build system and gcc into `/Build/esd`.
```
cmake -S . -B Build -G "Ninja"
cmake --build Build --config Release
```

*Yes: other toolchains and generators will work fine if you know what you're doing.*

### Why C++20?

This project makes use of the small subset of C++20 that is commonly supported by GCC, Clang and MSVC. I made this choice because I'm trying to get used to using C++20. The majority of code should be C++17 compatible although anything older would require significant refactoring (as I make use of `std::filesystem`).

As C++20 support improves: pull requests are welcome to continue modernizing this project. As ESD has no external dependencies and isn't intended to be consumed as a library, it seems like a great learning project for learning new C++ features so long as Linux, Windows and MacOS builds can still be produced easily.

### Tips

If you already have CMake installed as a Visual Studio feature you can easily setup a build script that finds that version of cmake rather than installing another copy. I recommend looking at [vssetup](https://github.com/microsoft/vssetup.powershell) to reliably get the visual studio install path.

### DIY

ESD is also designed to be simple to build. If your C++ compiler supports C++20 you can likely drag and drop the files from `/Source/*` into a build environment and build it with little to no configuration.

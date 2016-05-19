# Noxoscope

Noxoscope is a 3D real-time rendering program made for trying out different computer graphics techniques.

Techniques that have been implemented:
 - Deferred shading
   - Support for dynamically adding light sources
 - SSR, screen-space reflections
 - SSAO
 - Normal mapping

Features:
 - Many model formats are supported, using [assimp](http://www.assimp.org/)

## Screenshots

![Rendered sample scene](screenshots/render.png?raw=true)
Rendered sample scene

![Rendered sample scene with debug information](screenshots/render-debug.png?raw=true)
Rendered sample scene with debug information

## Building

Building requires CMake. See [cmake.org](https://cmake.org) for information about how to install.

### Windows

Building with Visual Studio 2015 is supported and has been tested.

#### Part 1 - Install SDL

Download the SDL2 development libraries for MSVC, e.g. `SDL2-devel-2.0.4-VC.zip`, at the SDL [download page](https://www.libsdl.org/download-2.0.php). 2.0.4 is the latest tested version. If a newer version is available, you could try using it. Once extracted somewhere, set the environment variable `SDL2` to that directory, which contains `include` and `lib` directories.

#### Part 2 - CMake generation

1. Create a directory `build` in the project root. (It can be named anything else and created anywhere else, but it is assumed to be in the project directory in the following step.)
2. From that directory, run `cmake -G "Visual Studio 14 2015 Win64" ..`, or another string if not using Visual Studio 2015. Compiling as 64-bit is recommended, but not required.

#### Part 3 - Link asset directory

Before running from within Visual Studio, some manual steps are needed to make the assets available from the build directory. Rather than copying assets from their original location by default, a symbolic link is created in the build directory. To distribute the program, you would need to manually copy the asset directory instead.

1. Open Noxoscope.sln in the build directory in Visual Studio.
2. Select Noxoscope as "StartUp Project" (by right-clicking on the Noxoscope project listed under the solution).
3. Compile the program.
4. A file `create_assets_link.bat` should now be available in the project root.
5. Run this batch script to create a symbolic link in the build directory. This may require running it as an administrator.

These steps needs to be done once for each build configuration (Debug/Release).

After this, it should be possible to compile and run the program.

### Linux

Compiling in a Debian derivative using make and Clang has been tested. (Using similar steps for building on OS X should be possible, but hasn't been tested.)

#### Part 1 - Install SDL

`libsdl2` is available in some Linux distributions. In Ubuntu, you can install the required dependencies it by running

```
sudo apt-get install libsdl2-dev
```

#### Part 2 - CMake generation

1. Create a directory `build` in the project root.
2. From that directory, run `cmake -G "<generator name>" ..`, where `<generator name>` is a [supported build system](https://cmake.org/cmake/help/v3.0/manual/cmake-generators.7.html). Leave out the -G argument to use the default.

#### Part 3 - Compile

If using a makefile project, run `make` in the build directory to compile. The executable `Noxoscope` should then be available.

### Tests

Run cmake with the additional argument `-DNS_BUILD_TESTS=ON`. For example, along with the previous Windows example, the final command would be

```
cmake -DNS_BUILD_TESTS=ON -G "Visual Studio 14 2015 Win64" ..
```

A single executable `NoxoscopeTest` for running all tests can now be built.

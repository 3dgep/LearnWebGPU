# Learn WebGPU

Source: https://eliemichel.github.io/LearnWebGPU/index.html
 
This project is intended to be used as a learning resource on using WebGPU with C++.

## Installing Dependencies

### CMake

### Ninja Build

```sh
winget install Ninja-build.Ninja
```

### Visual Studio 2022

> TODO: Figure out how to get Emscripten debugging working in Visual Studio.

### Visual Studio Code

#### Recommended Extensions

* C/C++ for Visual Studio Code
* CMake Tools
* Live Preview

### Emscripten

This project comes with Emscripten as a Git submodule.
We need to install and activate the version of Emscripten that will be used for this project.

```sh
git submodule update --init emsdk
cd emsdk
emsdk install 3.1.60
emsdk activate --permanent 3.1.60
```

## Building

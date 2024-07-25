# Learn WebGPU

> Source: [https://eliemichel.github.io/LearnWebGPU/index.html](https://eliemichel.github.io/LearnWebGPU/index.html)

This project is intended to be used as a learning resource on using WebGPU with C++.

## Installing Dependencies

### CMake

Download the latest version of CMake from [https://cmake.org/download](https://cmake.org/download/)

### Python



### Ninja Build

The Ninja build system is used to build the browser version of our application.

#### Windows

```sh
winget install Ninja-build.Ninja
```

#### Mac

```sh
brew install ninja
```

#### Linux

* Debian/Ubuntu

```sh
apt-get install ninja-build
```

See [Pre built Ninja packages](https://github.com/ninja-build/ninja/wiki/Pre-built-Ninja-packages) for more package managers.

### Visual Studio 2022

> TODO: Figure out how to get Emscripten debugging working in Visual Studio.

### Visual Studio Code

Visual Studio Code is better to use for building and debugging the browser builds.
There are several launch & tasks configurations to simplify debugging the application in the browser.

#### Recommended Visual Studio Code Extensions

This project includes an [extensions.json](.vscode/extensions.json) file for Visual Studio Code.

* C/C++ for Visual Studio Code
* CMake Tools
* Live Preview

### Emscripten

This project comes with Emscripten as a Git submodule.
We need to install and activate the version of Emscripten that will be used for this project.

Run the following commands to install and activate the version of Emscripten that is used for this project.

```sh
git submodule update --init tools/emsdk
cd tools/emsdk
.\emsdk install 3.1.64
.\emsdk activate --permanent 3.1.64
```

## Building

## Known Issues

* When using the Dawn WebGPU backend, the `fetch_dawn_dependencies.py` script can fail in cmake-gui with the error `OSError: [WinError 6] The handle is invalid`.
  * Solution: run cmake on the command line:
    * `cmake --preset vs17 -D WEBGPU_BACKEND=DAWN`

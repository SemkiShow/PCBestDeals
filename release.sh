#!/bin/bash

set -e

# Compiling for Linux
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
cp build/bin/asciify .

# Compiling for Windows
cmake -B build_win -DCMAKE_TOOLCHAIN_FILE=mingw-w64-x86_64.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build_win -j
cp build_win/bin/asciify.exe .

# Zipping the build
zip release.zip asciify asciify.exe ffmpeg ffmpeg.exe LICENSE README.md
rm asciify asciify.exe

# Creating a GitHub release
gh release create $1 release.zip
rm release.zip

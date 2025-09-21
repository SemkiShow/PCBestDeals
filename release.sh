#!/bin/bash

set -e
executable_name=PCBestDeals

# Compiling for Linux
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
cp build/bin/$executable_name .

# Compiling for Windows
cmake -B build_win -DCMAKE_TOOLCHAIN_FILE=mingw-w64-x86_64.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build_win -j
cp build_win/bin/$executable_name.exe .

# Zipping the build
zip release.zip $executable_name $executable_name.exe LICENSE README.md settings.txt
rm $executable_name $executable_name.exe

# Creating a GitHub release
gh release create $1 release.zip
rm release.zip

#!/bin/bash

# pcre2
cd /tmp
git clone --recursive https://github.com/PCRE2Project/pcre2.git --depth=1 --branch=pcre2-10.47
cd pcre2

cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/app/deps \
    -DPCRE2_BUILD_PCRE2_16=ON \
    -DPCRE2_BUILD_PCRE2_32=ON \
    -DPCRE2_SUPPORT_JIT=ON \
    -DPCRE2_STATIC_PIC=ON \
    -DPCRE2_BUILD_PCRE2GREP=OFF \
    -DPCRE2_BUILD_TESTS=OFF \
    -DCMAKE_DISABLE_FIND_PACKAGE_ZLIB=ON
cmake --build build -j$(nproc)
cmake --install build --strip

mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=RELEASE
make KaHyPar -j
make install.library

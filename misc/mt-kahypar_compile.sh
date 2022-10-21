mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=RELEASE
make MtKaHyPar -j
make install.mtkahypar

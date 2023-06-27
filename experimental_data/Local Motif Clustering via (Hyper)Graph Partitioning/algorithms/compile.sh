#!/bin/bash

rm -rf deploy
rm -rf build
mkdir build
cd build
cmake ../ -DCMAKE_BUILD_TYPE=Release $1
make -j 
cd ..

mkdir deploy
cp ./build/evaluator deploy/
cp ./build/motif_clustering_graph deploy/our_algorithm

rm -r ./build


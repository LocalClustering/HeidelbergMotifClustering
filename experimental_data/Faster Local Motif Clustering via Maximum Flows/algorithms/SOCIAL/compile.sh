#!/bin/bash

rm -rf deploy
rm -rf build
mkdir build
cd build
cmake ../ -DCMAKE_BUILD_TYPE=Release 
make -j 
cd ..

mkdir deploy
cp ./build/evaluator deploy/
cp ./build/triangle_counter deploy/
cp ./build/social deploy/

cd algorithms

# Compile our algorithm
./compile.sh

# Compile MAPPR
cd extern/snap/examples/localmotifcluster/
make
cd -;
cp extern/snap/examples/localmotifcluster/localmotifclustermain deploy/mappr

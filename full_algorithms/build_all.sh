# Compile MAPPR
cd extern/snap/examples/localmotifcluster/
make
cd -;
cp extern/snap/examples/localmotifcluster/localmotifclustermain deploy/mappr

# Compile (mt-)KaHyPar and KaMinPar (our algorithm used their libraries)
for i in mt-kahypar kahypar KaMinPar
do
	cd extern/$i;
	rm -r build
	./compile.sh;
	cd -;
done

# Compile our algorithm
./compile.sh;

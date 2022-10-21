########### PATCHING ###########

# Apply patches on external modules
for i in snap mt-kahypar kahypar KaMinPar
do
	cd extern/$i;
	git apply ../patches/${i}.patch
	cp ../../../misc/${i}_compile.sh ./compile.sh
	cd -;
done


########### COMPILING ###########

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


########### CLEANING ###########

# Apply patches on external modules
for i in snap mt-kahypar kahypar KaMinPar
do
	cd extern/$i;
	git checkout -fd
	cd -;
done



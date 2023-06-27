cd algorithms

########### PATCHING ###########

# Apply patches on external modules
for i in snap
do
	cd extern/$i;
	git apply ../patches/${i}.patch
	cp ../../../misc/${i}_compile.sh ./compile.sh
	cd -;
done


########### COMPILING ###########

# Compile our algorithm
./compile.sh

# Compile MAPPR
cd extern/snap/examples/localmotifcluster/
make
cd -;
cp extern/snap/examples/localmotifcluster/localmotifclustermain deploy/mappr


########### CLEANING ###########

# Remove patches from external modules
for i in snap
do
	cd extern/$i;
	git checkout -fd
	cd -;
done



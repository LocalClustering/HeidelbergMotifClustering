cd algorithms

########### PATCHING ###########

# Apply patches on external modules
cd MAPPR/;
git apply ../patches/snap.patch
cd -;


########### COMPILING ###########

# Compile SOCIAL
cd SOCIAL
./compile.sh
cd -;

# Compile LMCHGP
cd LMCHGP
./compile.sh
cd -;

# Compile MAPPR
cd MAPPR/examples/localmotifcluster/
make
cd -;

rm -r deploy
mkdir deploy
cp SOCIAL/deploy/* deploy/
cp LMCHGP/deploy/motif_clustering_graph deploy/lmchgp
cp MAPPR/examples/localmotifcluster/localmotifclustermain deploy/mappr

cd ..;


########### CLEANING ###########

# Remove patches from external modules
cd MAPPR;
git checkout -fd
cd -;



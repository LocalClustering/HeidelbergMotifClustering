# Clean MAPPR
cd extern/snap/examples/localmotifcluster/
rm localmotifclustermain
cd -;

# Clean (mt-)KaHyPar and KaMinPar (our algorithm used their libraries)
for i in mt-kahypar kahypar KaMinPar
do
	cd extern/$i;
	rm -r build
	cd -;
done

# Clean our algorithms
rm -r deploy

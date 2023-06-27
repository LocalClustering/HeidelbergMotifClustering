cd algorithms

# Clean collected binaries
rm -r deploy/

# Clean SOCIAL
cd SOCIAL
rm -r deploy/
rm -r build/
cd -;

# Clean LMCHGP
cd LMCHGP
rm -r deploy/
rm -r build/
cd -;

# Clean MAPPR
cd MAPPR/examples/localmotifcluster/
rm localmotifclustermain
cd -

cd ..

# Clean sample results
cd experiments/sample_experiments
rm -r `cat folders.txt`

# # Clean plots
# cd -
# cd plotting/plots
# rm -r stateoftheart/

cd algorithms

# Clean MAPPR
cd extern/snap/examples/localmotifcluster/
rm localmotifclustermain
cd -

# Clean our algorithm
rm -r deploy
cd ..

# Clean sample results
cd experiments/sample_experiments
rm -r `cat folders.txt`

# # Clean plots
# cd -
# cd plotting/plots
# rm -r stateoftheart/

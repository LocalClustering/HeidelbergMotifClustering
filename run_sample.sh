#############
echo Generate instances of experiments for the graph com-dblp
cd experiments/metascripts
for scr in gen_scripts_mappr.sh gen_scripts_ours.sh
do
	sh ${scr}
done
cd -

#############
echo
echo Run the generated experiments and collect results into a CSV file for each algorithm
cd experiments/sample_experiments
sh run.sh
sh collect.sh
cd -

#############
echo
echo Print average results
cd experiments/sample_experiments
sh print_statistics.sh

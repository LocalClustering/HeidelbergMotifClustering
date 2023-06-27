# DEFINE COLORS FOR PLOTS
colors="blue"
colors="${colors}:gray"
colors="${colors}:brown"
colors="${colors}:red"
colors="${colors}:green"
colors="${colors}:orange"
colors="${colors}:purple"
colors="${colors}:black"
colors="${colors}:light-blue"
colors="${colors}:pink"
colors="${colors}:magenta"
colors="${colors}:yellow"
colors="${colors}:red-magenta"

# MAKE COLOR NAMES ITERATABLE
colors=(${colors//:/ })

# CHECK WHICH CHARTS AXES ARE IN LOG SCALE
string_log2_res_pp=''
if [ "$log2_res_pp" = 1 ]; then
	string_log2_res_pp='set logscale x 2'
fi
string_log2_tim_pp=''
if [ "$log2_tim_pp" = 1 ]; then
	string_log2_tim_pp='set logscale x 2'
fi
if [ "$log10_tim_pp" = 1 ]; then
	string_log2_tim_pp='set logscale x 10'
fi

# CREATE UNIFIED CSV FILE
cvs_res="final_res_${plottheme}.txt"
cvs_tim="final_tim_${plottheme}.txt"
for i in ${collect}; do cat ${i}/results.txt; done >> ${cvs_res}
for i in ${collect}; do cat ${i}/times.txt; done >> ${cvs_tim}

# GIVE LOCATION OF SQLITE AND DEFINE LOCATION FOR TEMPORATY DATABASE
sgdb=`cat meta_scripts/sgdb.txt`
base=`cat meta_scripts/base.txt`

# GIVE LOCATION OF SQL GENERIC QUERIES
queries="queries"

# CREATE TABLES 
${sgdb} ${base} < ${queries}/create_tables.sql ; 

# EXPORT CSV FILES TO TABLES
${sgdb} ${base} -separator ' ' ".import ${cvs_res} results "
${sgdb} ${base} -separator ' ' ".import ${cvs_tim} times "

# CREATE HEADER OF EACH USED GNUPLOT SCRIPT
# Create folders to contain all plots
prefix="plots/${plottheme}"
mkdir -p ${prefix}/res
mkdir -p ${prefix}/tim
# create headers (each cat command below is associated with a different plot)
cat meta_scripts/pp_plotscript_res.gnuplot \
	| sed "s/generic_xrange/${xrange_res_pp}/g" \
	| sed "s/generic_xscale/${string_log2_res_pp}/g" \
	> ${prefix}/res/pp_plotscript.gnuplot
cat meta_scripts/double_plotscript.gnuplot \
	| sed "s/generic_xscale//g" \
	| sed "s/LabelY/Motif Conductance/g" \
	| sed "s/LabelX/Cluster Size/g" \
	> ${prefix}/res/double_plotscript.gnuplot
cat meta_scripts/pp_plotscript_tim.gnuplot \
	| sed "s/generic_xrange/${xrange_tim_pp}/g" \
	| sed "s/generic_xscale/${string_log2_tim_pp}/g" \
	> ${prefix}/tim/pp_plotscript.gnuplot

# RUN QUERIES ON SQLITE DATABASE AND ADD REMAINING LINES TO GNUPLOT SCRIPTS
# (Each cat command runs an SQL query. Each echo command adds a line to a gnuplot script)
counter=0
for item in $(echo ${caption} | tr "," " "); 
do 
	alg_code=(${item//:/ }) 
	cat ${queries}/perf_prof.sql \
		| sed "s/generic_table/results/g" \
		| sed "s/generic_objective/motif_conduc/g" \
		| sed "s/generic_algorithm/${alg_code[0]}/g" \
		| sed "s/generic_label/${alg_code[1]}/g" \
		| ${sgdb} ${base} -csv -separator '&' \
		> ${prefix}/res/pp_${alg_code[0]}.csv
	cat ${queries}/perf_prof.sql \
		| sed "s/generic_table/times/g" \
		| sed "s/generic_objective/time_clus/g" \
		| sed "s/generic_algorithm/${alg_code[0]}/g" \
		| sed "s/generic_label/${alg_code[1]}/g" \
		| ${sgdb} ${base} -csv -separator '&' \
		> ${prefix}/tim/pp_${alg_code[0]}.csv
	echo "'pp_${alg_code[0]}.csv' using '${alg_code[1]}':'ind' title '${alg_code[1]}' \
		with lines lw 9 linecolor rgb '${colors[$counter]}', \\" \
		>> ${prefix}/res/pp_plotscript.gnuplot
	echo "'pp_${alg_code[0]}.csv' using '${alg_code[1]}':'ind' title '${alg_code[1]}' \
		with lines lw 9 linecolor rgb '${colors[$counter]}', \\" \
		>> ${prefix}/tim/pp_plotscript.gnuplot
	cat ${queries}/list_two_objectives_all.sql \
		| sed "s/generic_table/results/g" \
		| sed "s/generic_objective1/motif_conduc/g" \
		| sed "s/generic_objective2/cluster_size/g" \
		| sed "s/generic_objective/motif_conduc/g" \
		| sed "s/generic_algorithm/${alg_code[0]}/g" \
		| sed "s/generic_label/${alg_code[1]}/g" \
		| ${sgdb} ${base} -csv -separator '&' \
		> ${prefix}/res/double_${alg_code[0]}.csv
	echo "'double_${alg_code[0]}.csv' using 'ind':'${alg_code[1]}' title '${alg_code[1]}' \
		with points lw 3 ps 1.8  linecolor rgb '${colors[$counter]}', \\" \
		>> ${prefix}/res/double_plotscript.gnuplot
	let counter=counter+1
done

# OBTAIN AND PRINT GENERAL STATISTICS IN CONSOLE
cat ${queries}/mean_stddev.sql \
	| sed "s/generic_table/results/g" \
	| sed "s/generic_objective/motif_conduc/g" \
	| sed "s/generic_decimal_places/3/g" \
	| ${sgdb} ${base} -table \
	> ${prefix}/res/pp_averages.csv
cat ${queries}/mean_stddev_geo.sql \
	| sed "s/generic_table/results/g" \
	| sed "s/generic_objective/cluster_size/g" \
	| sed "s/generic_decimal_places/0/g" \
	| ${sgdb} ${base} -table \
	>> ${prefix}/res/pp_averages.csv
cat ${queries}/mean_stddev_geo.sql \
	| sed "s/generic_table/times/g" \
	| sed "s/generic_objective/time_clus/g" \
	| sed "s/generic_decimal_places/2/g" \
	| ${sgdb} ${base} -table \
	> ${prefix}/tim/pp_averages.csv
cat ${prefix}/res/pp_averages.csv;
cat ${prefix}/tim/pp_averages.csv;
cat ${queries}/mean_per_graph.sql \
	| sed "s/generic_table/results/g" \
	| sed "s/generic_objective/motif_conduc/g" \
	| sed "s/generic_decimal_places/3/g" \
	| ${sgdb} ${base} -table 
cat ${queries}/mean_per_graph_geo.sql \
	| sed "s/generic_table/results/g" \
	| sed "s/generic_objective/cluster_size/g" \
	| sed "s/generic_decimal_places/0/g" \
	| ${sgdb} ${base} -table 
cat ${queries}/mean_per_graph_geo.sql \
	| sed "s/generic_table/times/g" \
	| sed "s/generic_objective/time_clus/g" \
	| sed "s/generic_decimal_places/2/g" \
	| ${sgdb} ${base} -table 

# RUN ALL GNUPLOT SCRIPTS
cd ${prefix}/res/
gnuplot pp_plotscript.gnuplot;
gnuplot double_plotscript.gnuplot;
cd -;
cd ${prefix}/tim/
gnuplot pp_plotscript.gnuplot;
cd -;

# DROP TABLES
${sgdb} ${base} < ${queries}/drop_tables.sql  

# DELETE UNIFIED CSV FILE
for i in $cvs_res $cvs_tim; do rm $i; done


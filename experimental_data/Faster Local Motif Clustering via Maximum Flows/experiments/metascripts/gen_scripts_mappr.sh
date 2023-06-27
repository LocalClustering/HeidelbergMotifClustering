home=`cd ../..; pwd`
evaluator="${home}/`cat ../location_evaluator.txt`"
program="${home}/`cat ../location_mappr.txt`"
database="${home}/`cat ../location_database.txt`"
graph_list=`cat ../graph_names.txt`
parameters=" "

n_seeds=50
n_repetitions=1

for graph in ${graph_list}
do
	triangles=`head -1 ${database}/${graph}.triangles | awk '{print $1}'` # Read triangle count from graph
	for diff_seed_nodes in `seq 1 ${n_seeds}`
	do
		u=`sed -n ${diff_seed_nodes}p ../seednodes/${graph}.seed` # read seed node in METIS format
		translated_seed=`sed -n ${u}p ${database}/${graph}.map`   # translate seed node to MAPPR format
		address="${home}/experiments/sample_experiments/mmappr"
		for i in `seq 1 ${n_repetitions}`
		do
			address2=${address}/$i/${graph}/$u/mmappr
			mkdir -p ${address2}
			echo "/usr/bin/time -v ${program} -d:N -i:${database}/${graph} -m:clique3 -s:${translated_seed} -o:${address2}/community -M:${database}/${graph}.map -w  ${parameters} 2> ${address2}/mem_result.txt 1> ${address2}/result.txt; ${evaluator} ${database}/${graph}.graph --input_partition=${address2}/community --triangle_count=${triangles} > ${address2}/eval_result.txt; rm ${address2}/community;  " > ${address2}/gen_map.sh
		done
	done
done


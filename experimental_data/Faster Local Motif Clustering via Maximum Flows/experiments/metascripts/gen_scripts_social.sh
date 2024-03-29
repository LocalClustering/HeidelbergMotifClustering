home=`cd ../..; pwd`
evaluator="${home}/`cat ../location_evaluator.txt`"
program="${home}/`cat ../location_social.txt`"
database="${home}/`cat ../location_database.txt`"
graph_list=`cat ../graph_names.txt`
parameters=" "


n_seeds=50
n_repetitions=1

for graph in ${graph_list}
do
	triangles=`head -1 ${database}/${graph}.triangles | awk '{print $1}'` # read seed node in METIS format
	for diff_seed_nodes in `seq 1 ${n_seeds}`
	do
		u=`sed -n ${diff_seed_nodes}p ../seednodes/${graph}.seed` # get seed node
		for alpha in 3
		do
			alpha_string="1"
			for next in `seq 2 ${alpha}`
			do
				alpha_string="${alpha_string}:${next}"
			done
			address="${home}/experiments/sample_experiments/ssocial"
			for i in `seq 1 ${n_repetitions}`
			do
				address2=${address}/$i/${graph}/$u/ssocial
				mkdir -p ${address2}

				echo "/usr/bin/time -v ${program} ${database}/${graph}.graph --seed_node=$u --seed=$i --bfsdepth_parameter_string=${alpha_string} --triangle_count=${triangles} --output_filename=${address2}/community  ${parameters} 2> ${address2}/mem_result.txt 1> ${address2}/result.txt; ${evaluator} ${database}/${graph}.graph --input_partition=${address2}/community --triangle_count=${triangles} > ${address2}/eval_result.txt; rm ${address2}/community; " > ${address2}/gen_map.sh
			done
		done
	done
done


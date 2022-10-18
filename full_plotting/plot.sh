# RELATIVE ADDRESS TO LOAD FILES results.txt AND times.txt
collect="data_results/stateoftheart/initials_int_mappr"
collect="${collect} data_results/stateoftheart/initials_fsocialintGraph_LPA3B80T1"
# collect="${collect} data_results/<further_group>/<further_set_of_tests>"

# THEME OF PLOTS
plottheme=stateoftheart

# CODE ASSOCIATED WITH BASELINE ALGORITHM FOR PLOTS
refalg=mappr

# MAP <CODE_OF_ALGORITHM,KEY>
caption="mmappr:MAPPR"
caption="${caption},our_algorithm:GL;3;80"
# caption="${caption},<code_of_further_set_of_tests>:<key_for_further_set_of_tests>"

# DEFINE RELATIVE POSITION OF CAPTIONS AND THE X-AXIS RANGE FOR PERFORMANCE PROFILES
xrange_res_pp="1:4"	     # range on x axis (motif conductance performance profile)
xrange_tim_pp="1:1024"	     # range on x axis (running time performance profile)
graph_check="com-orkut"	     # Graph used to plot Motif Conductance X Cluster Size
poscap_double="70,0.85"	     # Caption position in plot Motif Conductance X Cluster Size

#################### OPTIONAL PLOTTING OPTIONS ####################
speedup=1		     # plot speedup instead of time ratio
log2_res_pp=0		     # x-axis of motif conductance performance profile in log2 scale
log2_tim_pp=1		     # x-axis of running time performance profile in log2 scale

#################### NO NEED TO CHANGE BELOW HERE ####################
source ./meta_scripts/setup.sh


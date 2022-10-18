grep -rI 'Best Result' ./[-1-9]* | awk -F "[/ \t]" '{print $4, $2, $3, $5, $10, $12 }' | sort > parameters.txt;
# index seednode seed graph algorithm depth imbalance 

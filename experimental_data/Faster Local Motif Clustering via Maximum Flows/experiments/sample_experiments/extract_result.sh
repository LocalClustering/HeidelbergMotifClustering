grep -rI 'Best Result' ./[-1-9]* | awk -F "[/ \t]" '{print $4, $2, $3, $5, $15 }' | sort > tmp1.txt;
grep -rI 'Size of cluster' ./[-1-9]* | awk -F "[/ \t]" '{print $4, $2, $3, $5, $9 }' | sort > tmp2.txt;
paste tmp1.txt tmp2.txt | awk -F "[/\t ]" '{print 0, $1, $2, $3, $4, $5, $10 }' > results.txt;
# index seednode seed graph algorithm motif_conductance cluster_size 

rm tmp*.txt

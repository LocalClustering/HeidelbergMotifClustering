grep -rI 'Clustering Time' ./[-1-9]* | awk -F "[/ \t]" '{print $4, $2, $3, $5, $9 }' | sort > tmp1.txt;
grep -rI 'Enumeration Time' ./[-1-9]* | awk -F "[/ \t]" '{print $4, $2, $3, $5, $10 }' | sort > tmp2.txt;
paste tmp1.txt tmp2.txt | awk -F "[/\t ]" '{print 0, $1, $2, $3, $4, $5, $10 }' > times.txt;
# index seednode seed graph algorithm clustering_time enum_time 

rm tmp*.txt

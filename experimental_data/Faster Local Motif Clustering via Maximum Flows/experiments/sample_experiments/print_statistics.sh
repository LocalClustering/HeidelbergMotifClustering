for address in `cat folders.txt`;
do
	cd $address
	echo
	echo $address
	echo avg motif conductance: `cat results.txt | awk 'BEGIN{C=0;B=0}$4=="com-dblp"{C+=($6);D++;}END{print (C/D)}'`
	echo avg cluster size: `cat results.txt | awk 'BEGIN{C=0;B=0}$4=="com-dblp"{C+=log($7);D++;}END{print exp(C/D)}'`
	echo avg clustering time: `cat times.txt | awk 'BEGIN{C=0;B=0}$4=="com-dblp"{C+=log($6);D++;}END{print exp(C/D)}'`
	cd ..
done


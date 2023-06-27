for address in `cat folders.txt`;
do
	cp extract_* $address
	cd $address
	for i in extract_*
	do
		sh $i
	done
	cd ..
done


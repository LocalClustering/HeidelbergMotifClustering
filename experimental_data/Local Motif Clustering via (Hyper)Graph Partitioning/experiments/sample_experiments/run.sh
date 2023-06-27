for address in `cat folders.txt`;
do
	find ./$address -iname gen_map.sh | shuf > ./${address}/files.txt;
	cat ./${address}/files.txt;
done | parallel --bar  -P4 sh

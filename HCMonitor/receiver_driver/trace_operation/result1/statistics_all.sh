#/bin/bash
rm -f frequency item frequency_tmp
for((i=62; i<=64; i++))
do
	file_name=tmp$i.trace.analyzed
	cat $file_name | grep -E "*000" | sort -n | uniq > item
	cat item | ( while read arg; do cat $file_name | grep $arg | wc -l; done ) >> frequency_tmp
	cat frequency_tmp | sort -n > frequency
	 
	paste item frequency > $file_name.result
	rm -f frequency item frequency_tmp
done

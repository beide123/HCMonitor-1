#/bin/bash
	file_name=$1
	cat $file_name | grep -E "*000" | sort | uniq > item
	cat item | ( while read arg; do cat $file_name | grep $arg | wc -l; done ) >> frequency 
	paste item frequency > $file_name.result
	rm -f frequency item 

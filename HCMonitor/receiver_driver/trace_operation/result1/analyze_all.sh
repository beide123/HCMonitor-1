for((i=62; i<=64; i++)) 
do
	./trace_analyze_addr_wzp tmp$i.trace tmp$i.trace.analyzed 0 > 	tmp$i.trace.analyzed.tmp
	cat tmp$i.trace.analyzed.tmp | awk '{ print $5 }' > tmp$i.trace.analyzed
	rm -f tmp$i.trace.analyzed.tmp 
done

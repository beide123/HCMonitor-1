for((i=0; i<=4; i++)) 
do
	./trace_analyze_addr_wzp bug$i.trace bug$i.trace.analyzed 10000 > 	bug$i.trace.analyzed.bug
	cat bug$i.trace.analyzed.bug | awk '{ print $5 }' > bug$i.trace.analyzed
	rm -f bug$i.trace.analyzed.bug 
done

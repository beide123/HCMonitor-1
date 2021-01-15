./trace_analyze_addr_wzp tmp$1.trace tmp$1.trace.analyzed $2 > 	tmp$1.trace.analyzed.tmp
cat tmp$1.trace.analyzed.tmp | awk '{ print $5 }' > tmp$1.trace.analyzed.one
rm -f tmp$1.trace.analyzed.tmp 

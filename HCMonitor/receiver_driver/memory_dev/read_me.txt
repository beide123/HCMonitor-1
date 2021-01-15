1 初始化准备

   make

   ./memory_dev_load

2 收集trace

   gcc -o trace_collect collect_kerner_trace.c

   ./trace_collect filename

3 分析trace

   gcc -o trace_analyze trace_analyze_huawei.c

   ./trace_analyze filename rw 0 all n
   分析结果与原始trace存放在相同的路径下
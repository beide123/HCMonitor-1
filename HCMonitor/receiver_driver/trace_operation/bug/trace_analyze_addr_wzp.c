#include<stdio.h>
#include<string.h>
#include<stdlib.h>
//#define number_type unsigned long long
#define number_type int64_t

typedef struct Field
{
	int begin;
	int end;
} Field;

struct Field Type;
struct Field Seq_no;
struct Field Trace_dif;
struct Field Trace_r_w;
struct Field Trace_addr;
struct Field Global_counter_seq;
struct Field Global_counter;

number_type type;
number_type seq_no;
number_type trace_dif;
number_type trace_r_w;
number_type trace_addr;
number_type global_counter_seq;
number_type global_counter;

int assign_Field(Field *p, int begin, int end)
{
	p->begin = begin;
	p->end	 = end;
	return 0;
}

int ini_Fields()
{
	assign_Field(&Type, 63, 62 ); 
	assign_Field(&Seq_no, 61, 52); 
	assign_Field(&Trace_dif, 51, 32);
	assign_Field(&Trace_r_w, 31, 31);
	assign_Field(&Trace_addr, 30, 0); 
	assign_Field(&Global_counter_seq, 51, 45 ); 
	assign_Field(&Global_counter, 44, 0); 
}

number_type get_field(number_type *p, Field f)
{
	number_type right_shift = f.end;
	number_type mask	= (1ULL<<(f.begin - f.end + 1)) - 1;
	return (number_type)(((*p)>>right_shift) & mask);
}


//number_type type;
//number_type seq_no;
//number_type trace_dif;
//number_type trace_addr;
//number_type global_counter_seq;
//number_type global_counter;

int trace_decode(number_type *p)
{
	number_type type = get_field(p, Type);
	seq_no = get_field(p, Seq_no);
	if (type == 0)
	{
		trace_dif = get_field(p, Trace_dif);
		trace_r_w = get_field(p, Trace_r_w);
		trace_addr = get_field(p, Trace_addr);
	}
	else if ((type == 1) || (type == 2))
	{
		global_counter_seq = get_field(p, Global_counter_seq);
		global_counter	= get_field(p, Global_counter);
	}
	else 
	{
		printf("Error, the type value of this trace is wrong!\n");
	}
}

int my_printf(number_type *p)
{	
//	printf("%llx\n", *p);
	if (type == 0)
	{
		printf("%llu %llu %llu %llu %llx\n", type, seq_no, trace_dif, trace_r_w, trace_addr);	
	}
	else if ((type == 1) || (type == 2))
	{
	//	printf("%llu %llu %llu %llu\n", type, seq_no, trace_dif, global_counter_seq, global_counter);	
	}
	else 
	{
		printf("Error, the type value of this trace is wrong!\n");
	}
	//getchar();
}
int main(int argc, char **argv)
{
    char filename[256];
    char outputFilename[256];
	int analyze_count = 0;
    FILE *fp;
	
	number_type buf;

	ini_Fields();
    memset(filename, 0, 256);
    memset(outputFilename, 0, 256);
    if(argc != 5)
    {
        printf("./%s filename\n",argv[0]);
        return 0;
    }
    strcpy(filename, argv[1]);
    strcpy(outputFilename, argv[2]);
	analyze_count = atoi(argv[3]);
    if((fp = fopen(filename, "r")) == NULL)
    {
        printf("Open file %s error\n",filename);
        return 0;
    }
	
//    if((fpOut = fopen(outputFilename, "w+")) == NULL)
//    {
//        printf("Open output file error\n");
//        return 0;
//    }
//
//    fprintf(fpOut,"The min value is%d\n",min);
//    //malloc the tmp buf for trace
//	buf = (int64_t*)malloc(TRACE_LENGTH*NUM);
//	addrList = (unsigned*)malloc(4* sizeof(unsigned));
//	raddrList=(unsigned*)malloc(4* sizeof(unsigned));
//	memset(addrList,0,4*sizeof(unsigned));
//	memset(raddrList,0,4*sizeof(unsigned));
//    if(!raddrList || !addrList) 
//    {
//        printf("malloc failed\n");
//        return 0;
//    }
//

    //trace analyze
    	
	//while(!feof(fp))
	int i = 0;
	while(i < analyze_count)
	{
		fread(&buf, 8*sizeof(char), 1, fp);
		trace_decode(&buf);
		my_printf(&buf);
		if (strcmp(argv[4], "stop_per_trace") == 0)
			getchar();
		i++;
	}

    	fclose(fp);

    return 1;

}







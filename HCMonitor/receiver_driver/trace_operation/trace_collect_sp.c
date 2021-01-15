#define _GNU_SOURCE
#include <asm/unistd.h> 
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sched.h>
#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>

//#define _GNU_SOURCE

#if 1
#define WRITE write
#else
int WRITE(int handle, void *buf, int nbyte){
	return nbyte;
}
#endif

#define	 DMA
//#include "../GetCfgContent.h"
#include "../cfg_content.h"

#define phys_to_virtual(phys) (phys - DMA_CONTROL_REGISTER + memory_dev_addr)


char *memory_dev_addr;
extern int errno;

volatile unsigned long long *ptr_writing_addr;
unsigned long long *ptr_stopt_addr;
//unsigned long long  writing_addr;
unsigned long long  reading_addr;
unsigned long long  stopt_addr;

unsigned long long glb_writing_addr;// = DMA_BUFFER_ADDRESS;
unsigned long long glb_reading_addr;// = DMA_BUFFER_ADDRESS-1;

unsigned long long writing_size;
unsigned long long reading_size;
struct timeval	tvafter_writing, tvpre_writing;
struct timeval	tvafter_reading, tvpre_reading;
struct timezone tz;
double tv_writing=0;
double tv_reading=0;
double writing_speed;
double reading_speed;

double sum_tv_writing=0;
struct timeval	tvlast_writing, tvfirst_writing;
char datetime[200];

int direct_io = 0;
int flag = O_RDWR|O_CREAT|O_TRUNC;

char *buffer2 = NULL;
char *originbuffer2 = NULL;
int usebuffer2 = 0;
typedef unsigned long long u64;

int overflow = 0;

int fp;

#if 1
void RT_read_report(unsigned long long reading_size){
	reading_speed = (double)reading_size/tv_reading/1024/1024;
	//printf("					the store speed is %7.2f MiB/s.\r",reading_speed);
	//fflush(stdout);
}
#else
#define RT_read_report(...) 
#endif

#if 1
void RT_write_report(unsigned long long writing_size){
	writing_speed = (double)writing_size/tv_writing/1024/1024;		//in MB/s
	//printf("$ The trace bandwidth is %7.2f MiB/s;									\r",writing_speed);
	//fflush(stdout);
	sum_tv_writing += tv_writing;
}
#else
#define RT_write_report(...) 
#endif

void gettime(){
	time_t now;
	struct tm *tm_now;
	
	time(&now);
	tm_now = localtime(&now);
	strftime(datetime, 200, "%Y-%m-%d %H:%M:%S", tm_now);
}

//check the writing pointer to see whether overflow happens.

void *check_overflow()
{
	//maintain the writing_addr
	
	unsigned long long  writing_addr;
	unsigned long long  last_writing_addr = DMA_BUFFER_ADDRESS;

	gettimeofday(&tvpre_writing,&tz);
	tvfirst_writing = tvpre_writing;
	
	while(writing_addr = *ptr_writing_addr)
	{
		if(writing_addr != last_writing_addr)
		{	//update the glb_writing_addr;
			gettimeofday(&tvafter_writing,&tz);
			writing_size = (writing_addr - last_writing_addr + DMA_BUFFER_TOTAL_SIZE) % DMA_BUFFER_TOTAL_SIZE;
			tv_writing = (double)(tvafter_writing.tv_sec - tvpre_writing.tv_sec) + (double)(tvafter_writing.tv_usec - tvpre_writing.tv_usec)/1000000;
			RT_write_report(writing_size);
			
			last_writing_addr = writing_addr;
			glb_writing_addr += writing_size;
			gettimeofday(&tvpre_writing,&tz);
			
			if(glb_writing_addr > glb_reading_addr + DMA_BUFFER_TOTAL_SIZE)
			{
				overflow ++;
				printf("glb_writing_addr %10.2f, glb_reading_addr %10.2f\n",(double)glb_writing_addr/1024/1024,(double)glb_reading_addr/1024/1024);
				pthread_exit(NULL);
				//overflow ++;
				//printf("glb_writing_addr %10.2f, glb_reading_addr %10.2f\n",(double)glb_writing_addr/1024/1024,(double)glb_reading_addr/1024/1024);
				//glb_writing_addr -= DMA_BUFFER_TOTAL_SIZE;
				//printf("glb_writing_addr %10.2f, glb_reading_addr %10.2f\n",(double)glb_writing_addr/1024/1024,(double)glb_reading_addr/1024/1024);
			//	printf("\n%d\n",overflow);
			}
		}
		usleep(10000);//10ms,it will take at least 80ms to fillin a 64MB segment at the speed of 800MB/s
	}
	
	gettimeofday(&tvafter_writing,&tz);
	tvlast_writing = tvafter_writing;
	stopt_addr = *ptr_stopt_addr;
	writing_size = (stopt_addr - last_writing_addr + DMA_BUFFER_TOTAL_SIZE) % DMA_BUFFER_TOTAL_SIZE;
	tv_writing = (double)(tvafter_writing.tv_sec - tvpre_writing.tv_sec) + (double)(tvafter_writing.tv_usec - tvpre_writing.tv_usec)/1000000;
	RT_write_report(writing_size);
	
	glb_writing_addr += writing_size;
	
	if(glb_writing_addr > glb_reading_addr + DMA_BUFFER_TOTAL_SIZE)
	{
		overflow++;
		pthread_exit(0);
		//overflow ++;
		//printf("glb_writing_addr %10.2f, glb_reading_addr %10.2f\n",(double)glb_writing_addr/1024/1024,(double)glb_reading_addr/1024/1024);
		//glb_writing_addr -= DMA_BUFFER_TOTAL_SIZE;
		//printf("glb_writing_addr %10.2f, glb_reading_addr %10.2f\n",(double)glb_writing_addr/1024/1024,(double)glb_reading_addr/1024/1024);
	}
}

void *store_to_disk()
{
	unsigned long long tmp;
	int i = 0;
	int j;
	char *curbuffer;
	int cur_reading_size;

	unsigned long long  writing_addr;
	reading_addr = DMA_BUFFER_ADDRESS-1;
	
	//maintain the reading_addr
	while(writing_addr = *ptr_writing_addr)
	{
		if(writing_addr == reading_addr + 1)
		{
			usleep(10000);
			continue;
		}
		else if(writing_addr > reading_addr + 1)
		{
			gettimeofday(&tvpre_reading,&tz);
			tmp = writing_addr;
			reading_size = writing_addr - reading_addr - 1;
			
			for(cur_reading_size = 0; cur_reading_size < reading_size; cur_reading_size += DMA_BUFFER_SEGMENT_SIZE){
				curbuffer = phys_to_virtual(reading_addr)+1+cur_reading_size;
				if(usebuffer2){
					for(j = 0; j < DMA_BUFFER_SEGMENT_SIZE; j+= 8){
						*((u64 *)(buffer2 + j)) = *((u64 *)(curbuffer + j));
					}
					curbuffer = buffer2;
				}
				if((i = WRITE( fp,curbuffer, DMA_BUFFER_SEGMENT_SIZE)) != DMA_BUFFER_SEGMENT_SIZE)
					printf("\n  #a:WARNINGing: write error!,%d,%d\n",i,errno);
				if(reading_size - cur_reading_size < DMA_BUFFER_SEGMENT_SIZE)
					printf("\n  #a:WARNINGing: reading_size - cur_reading_size < DMA_BUFFER_SEGMENT_SIZE!\n");
			}
			//if((i = WRITE( fp,phys_to_virtual(reading_addr)+1, reading_size)) != reading_size)
			//	printf("\n  #a:WARNINGing: write error!,%d,%d\n",i,errno);

			gettimeofday(&tvafter_reading,&tz);
			tv_reading = (double)(tvafter_reading.tv_sec - tvpre_reading.tv_sec) + (double)(tvafter_reading.tv_usec - tvpre_reading.tv_usec)/1000000;
			RT_read_report(reading_size);

			glb_reading_addr += reading_size;
		}
		else
		{
			gettimeofday(&tvpre_reading,&tz);
			tmp = DMA_BUFFER_ADDRESS;
			reading_size = DMA_BUFFER_ADDRESS + DMA_BUFFER_TOTAL_SIZE - reading_addr - 1;

			for(cur_reading_size = 0; cur_reading_size < reading_size; cur_reading_size += DMA_BUFFER_SEGMENT_SIZE){
				curbuffer = phys_to_virtual(reading_addr)+1+cur_reading_size;
				if(usebuffer2){
					for(j = 0; j < DMA_BUFFER_SEGMENT_SIZE; j+= 8){
						*((u64 *)(buffer2 + j)) = *((u64 *)(curbuffer + j));
					}
					curbuffer = buffer2;
				}
				if((i = WRITE( fp,curbuffer, DMA_BUFFER_SEGMENT_SIZE)) != DMA_BUFFER_SEGMENT_SIZE)
					printf("\n  #b:WARNINGing: write error!,%d,%d\n",i,errno);
				if(reading_size - cur_reading_size < DMA_BUFFER_SEGMENT_SIZE)
					printf("\n  #b:WARNINGing: reading_size - cur_reading_size < DMA_BUFFER_SEGMENT_SIZE!\n");
			}

			//if((i = WRITE( fp,phys_to_virtual(reading_addr)+1, reading_size)) != reading_size)
			//	printf("\n  #b:WARNINGing: write error!,%d,%d\n",i,errno);

			gettimeofday(&tvafter_reading,&tz);
			tv_reading = (double)(tvafter_reading.tv_sec - tvpre_reading.tv_sec) + (double)(tvafter_reading.tv_usec - tvpre_reading.tv_usec)/1000000;
			RT_read_report(reading_size);

			glb_reading_addr += reading_size;
			
			//if((writing_addr = *ptr_writing_addr) && (writing_addr != DMA_BUFFER_ADDRESS))
			{
				gettimeofday(&tvpre_reading,&tz);
				tmp = writing_addr;
				reading_size = writing_addr - DMA_BUFFER_ADDRESS;

				for(cur_reading_size = 0; cur_reading_size < reading_size; cur_reading_size += DMA_BUFFER_SEGMENT_SIZE){
					curbuffer = phys_to_virtual(DMA_BUFFER_ADDRESS)+cur_reading_size;
					if(usebuffer2){
						for(j = 0; j < DMA_BUFFER_SEGMENT_SIZE; j+= 8){
							*((u64 *)(buffer2 + j)) = *((u64 *)(curbuffer + j));
						}
						curbuffer = buffer2;
					}
					if((i = WRITE( fp,curbuffer, DMA_BUFFER_SEGMENT_SIZE)) != DMA_BUFFER_SEGMENT_SIZE)
						printf("\n  #c:WARNINGing: write error!,%d,%d\n",i,errno);
					if(reading_size - cur_reading_size < DMA_BUFFER_SEGMENT_SIZE)
						printf("\n  #c:WARNINGing: reading_size - cur_reading_size < DMA_BUFFER_SEGMENT_SIZE!\n");
				}
				//if((i = WRITE( fp,phys_to_virtual(DMA_BUFFER_ADDRESS), reading_size)) != reading_size)
				//	printf("\n  #c:WARNINGing: write error!,%d,%d\n",i,errno);

				gettimeofday(&tvafter_reading,&tz);
				tv_reading = (double)(tvafter_reading.tv_sec - tvpre_reading.tv_sec) + (double)(tvafter_reading.tv_usec - tvpre_reading.tv_usec)/1000000;
				RT_read_report(reading_size);
				
				glb_reading_addr += reading_size;
			}
		}
		reading_addr = tmp - 1;
	}
		
	if(*ptr_stopt_addr > reading_addr + 1)
	{
		gettimeofday(&tvpre_reading,&tz);
		reading_size = *ptr_stopt_addr - reading_addr - 1;
		if(direct_io) reading_size = (reading_size + 511ull) & ~511ull;
		for(cur_reading_size = 0; cur_reading_size < reading_size; cur_reading_size += DMA_BUFFER_SEGMENT_SIZE){
			curbuffer = phys_to_virtual(reading_addr)+1+cur_reading_size;
			if(usebuffer2){
				for(j = 0; j < DMA_BUFFER_SEGMENT_SIZE; j+= 8){
					*((u64 *)(buffer2 + j)) = *((u64 *)(curbuffer + j));
				}
				curbuffer = buffer2;
			}
			if((i = WRITE( fp,curbuffer, DMA_BUFFER_SEGMENT_SIZE)) != DMA_BUFFER_SEGMENT_SIZE)
				printf("\n  #1:WARNINGing: write error!,%d,%d\n",i,errno);
			//if(reading_size - cur_reading_size < DMA_BUFFER_SEGMENT_SIZE)
			//	printf("\n  #1:WARNINGing: reading_size - cur_reading_size < DMA_BUFFER_SEGMENT_SIZE!\n");
		}
		//if((i = WRITE( fp, phys_to_virtual(reading_addr)+1, reading_size)) != reading_size)
		//	printf("\n  #1:WARNINGing: write error!,%d,%d,%d\n",i,errno,reading_size);
		gettimeofday(&tvafter_reading,&tz);
		tv_reading = (double)(tvafter_reading.tv_sec - tvpre_reading.tv_sec) + (double)(tvafter_reading.tv_usec - tvpre_reading.tv_usec)/1000000;
		RT_read_report(reading_size);
		
		glb_reading_addr += reading_size;
	}
	else if(*ptr_stopt_addr < reading_addr + 1)
	{
		gettimeofday(&tvpre_reading,&tz);
		reading_size = DMA_BUFFER_ADDRESS + DMA_BUFFER_TOTAL_SIZE - reading_addr - 1;
		for(cur_reading_size = 0; cur_reading_size < reading_size; cur_reading_size += DMA_BUFFER_SEGMENT_SIZE){
			curbuffer = phys_to_virtual(reading_addr)+1+cur_reading_size;
			if(usebuffer2){
				for(j = 0; j < DMA_BUFFER_SEGMENT_SIZE; j+= 8){
					*((u64 *)(buffer2 + j)) = *((u64 *)(curbuffer + j));
				}
				curbuffer = buffer2;
			}
			if((i = WRITE( fp,curbuffer, DMA_BUFFER_SEGMENT_SIZE)) != DMA_BUFFER_SEGMENT_SIZE)
				printf("\n  #d:WARNINGing: write error!,%d,%d\n",i,errno);
			if(reading_size - cur_reading_size < DMA_BUFFER_SEGMENT_SIZE)
				printf("\n  #d:WARNINGing: reading_size - cur_reading_size < DMA_BUFFER_SEGMENT_SIZE!\n");
		}
		//if((i = WRITE( fp, phys_to_virtual(reading_addr)+1, reading_size)) != reading_size)
		//	printf("\n  #d:WARNINGing: write error!,%d,%d\n",i,errno);
		tv_reading = (double)(tvafter_reading.tv_sec - tvpre_reading.tv_sec) + (double)(tvafter_reading.tv_usec - tvpre_reading.tv_usec)/1000000;
		RT_read_report(reading_size);
		
		glb_reading_addr += reading_size;
		
		gettimeofday(&tvpre_reading,&tz);
		reading_size = *ptr_stopt_addr - DMA_BUFFER_ADDRESS;
		if(direct_io) reading_size = (reading_size + 511ull) & ~511ull;
		//printf("direct_io:%d",direct_io);
		for(cur_reading_size = 0; cur_reading_size < reading_size; cur_reading_size += DMA_BUFFER_SEGMENT_SIZE){
			curbuffer = phys_to_virtual(DMA_BUFFER_ADDRESS)+cur_reading_size;
			if(usebuffer2){
				for(j = 0; j < DMA_BUFFER_SEGMENT_SIZE; j+= 8){
					*((u64 *)(buffer2 + j)) = *((u64 *)(curbuffer + j));
				}
				curbuffer = buffer2;
			}
			if((i = WRITE( fp,curbuffer, DMA_BUFFER_SEGMENT_SIZE)) != DMA_BUFFER_SEGMENT_SIZE)
				printf("\n  #2:WARNINGing: write error!,%d,%d\n",i,errno);
			//if(reading_size - cur_reading_size < DMA_BUFFER_SEGMENT_SIZE)
			//	printf("\n  #2:WARNINGing: reading_size - cur_reading_size < DMA_BUFFER_SEGMENT_SIZE!\n");
		}
		//if((i = WRITE( fp, phys_to_virtual(DMA_BUFFER_ADDRESS), reading_size)) != reading_size)
		//	printf("\n  #2:WARNINGing: write error!,%d,%d,%d\n",i,errno,reading_size);
		tv_reading = (double)(tvafter_reading.tv_sec - tvpre_reading.tv_sec) + (double)(tvafter_reading.tv_usec - tvpre_reading.tv_usec)/1000000;
		RT_read_report(reading_size);
		
		glb_reading_addr += reading_size;
	}
	//printf("\r\033[K\033[2A");
	//fflush(stdout);
}


int main(int argc, char **argv)
{
	char filename[64];
	int i = 0;
	
	if(argc < 2)
	{
		printf("Usage:./trace_collect filename [-I]\n");
		return 0;
	}
	/*if(argc > 2){
		if(!strcmp(argv[2], "-I")){
			direct_io = 1;
			flag |= O_DIRECT;
		}
	}*/
	for(i = 2;i < argc; i++){
		if(argv[i][0] == '-'){
			switch(argv[i][1]){
			case 'I':
				direct_io = 1;
				flag |= O_DIRECT;
				break;
			case 'B':
				usebuffer2 = 1;
				break;
			default:
				return 0;
			}
		}
		//else{
		//	strcpy(filename, argv[i]);
		//}
	}
	printf("Use %s\n\n", direct_io ? "Direct IO" : "Buffered IO");
	printf("Use %s\n\n", usebuffer2 ? "Cache" : "unCache");

	memset(filename, 0, 64);
	strcpy(filename, argv[1]);
	strcat(filename, ".trace");
	//if((fp = open(filename,O_RDWR|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)) < 0)
	if((fp = open(filename,flag,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)) < 0)
	{
		printf("Open file %s error\n", filename);
		return 0;
	}

	//printf("page_size is %d\n", getpagesize());
	/*
	*  Step.1 Initial the DMA buffer for trace collection
	*/	

	int ff; //the file descriptor of the device memory_dev 
	unsigned long long memory_dev_size;	//the size of DMA buffer plus the DMA control registers
	
	ff = open("/dev/memory_dev", O_RDWR);
	if( ff < 0 ){
		printf("open /dev/memory_dev failed\n");
		return 0;
	}

	if(usebuffer2 && (originbuffer2 = (char *)malloc(DMA_BUFFER_SEGMENT_SIZE * sizeof(char) + 512)) == 0){
		printf("malloc originbuffer2 failed\n");
		return 0;
	}
	buffer2 = (char *)((unsigned long long)(originbuffer2 + 511ull) & ~511ull);
		
	//GetCfgContent();	//get the DMA information from ../cfg_content
	
	memory_dev_size = DMA_BUFFER_ADDRESS - DMA_CONTROL_REGISTER + DMA_BUFFER_TOTAL_SIZE;
	memory_dev_addr = mmap(0, memory_dev_size, PROT_READ|PROT_WRITE, MAP_SHARED,ff,0);
	printf("memset start.\n");
	memset(memory_dev_addr, 0, memory_dev_size);
	printf("memset over.\n");
	if ((unsigned long long)memory_dev_addr< 0)
	{
		printf("error in mmap\n");
		switch(errno)
		{
			case EACCES: printf("EACCES\n"); break;
			case EAGAIN: printf("EAGAIN\n"); break;
			case EBADF: printf("EBADF\n"); break;
			case EINVAL: printf("EINVAL\n"); break;
			case ENFILE: printf("ENFILE\n"); break;
			case ENODEV: printf("ENODEV\n"); break;
			case ENOMEM: printf("ENOMEM\n"); break;
			case EPERM: printf("EPERM\n"); break;
		}
		return -1;
	}

	unsigned long long remain;	
	gettimeofday(&tvpre_reading,&tz);
	remain = 0;
	while(remain < memory_dev_size){
		*((unsigned long long *)((char *)memory_dev_addr + remain)) += 1ull;
		remain += 8;
	}

	gettimeofday(&tvafter_reading,&tz);

	tv_reading = (double)(tvafter_reading.tv_sec - tvpre_reading.tv_sec) + (double)(tvafter_reading.tv_usec - tvpre_reading.tv_usec)/1000000;
	reading_speed = (double)reading_size/tv_reading/1024/1024;
	//printf("unsigned long long : %7.2fMB/s\n",(double)memory_dev_size/tv_reading/1024/1024);
	
	/*
	*  Step.2 Read the traces from DMA buffer and store them into file
	*/	
	
	ptr_writing_addr = (unsigned long long *)memory_dev_addr;
	ptr_stopt_addr   = (unsigned long long *)(memory_dev_addr + 8);
	
	//set the writing_addr to zero at the beginning;
	*ptr_writing_addr = 0;
    
	//printf("\n\033[31;5mspecial mode\033[0m\n");
	printf("\n Waiting for trace...\n");
    
	while(*ptr_writing_addr == 0)
	{
		usleep(10000);
	}
	
	printf("\n Collecting trace...\n\n");    
   
	 
	glb_writing_addr = DMA_BUFFER_ADDRESS;
	glb_reading_addr = DMA_BUFFER_ADDRESS-1;

	//create two threads: one to check overflow; one to store traces into file
	pthread_t	tid_1,tid_2;
	
	pthread_create(&tid_1, NULL, check_overflow, NULL);
	pthread_create(&tid_2, NULL, store_to_disk, NULL);
	pthread_join(tid_1,NULL);
	if(!overflow) 
	{
		pthread_join(tid_2,NULL);

		printf("\n\n Done!");
		printf("  %10.2fMiB of trace collected.\n\n",(double)(glb_reading_addr-DMA_BUFFER_ADDRESS+1)/1024/1024);
		//printf("  %llu, %llu\n",glb_writing_addr-DMA_BUFFER_ADDRESS+1,glb_reading_addr-DMA_BUFFER_ADDRESS+1);
		if(glb_writing_addr != glb_reading_addr && ftruncate(fp, glb_writing_addr-DMA_BUFFER_ADDRESS+1)){
			printf("ftruncate failed\n");
		}
		tv_writing = (double)(tvlast_writing.tv_sec - tvfirst_writing.tv_sec) + (double)(tvlast_writing.tv_usec - tvfirst_writing.tv_usec)/1000000;
		writing_speed = (double)(glb_writing_addr-DMA_BUFFER_ADDRESS+1)/tv_writing/1024/1024;		//in MB/s
		double writing_speed_1 = (double)(glb_writing_addr-DMA_BUFFER_ADDRESS+1)/sum_tv_writing/1024/1024;
		//printf("%7.2fMB/s, %7.2fMB/s, overhead %%%.2f\n", writing_speed_1, writing_speed, (tv_writing - sum_tv_writing) / tv_writing * 100);
		printf("  Throughput: %7.2fMiB/s\n", writing_speed);
	}
	else	
		printf("\n\n # ERROR: OVERFLOW HAPPENED!\n\n");
	//if(overflow) printf("  Overflow happened! %7.2f MB of trace lost!\n\n",(double)overflow*DMA_BUFFER_TOTAL_SIZE/1024/1024);
	printf("\n");
	munmap(memory_dev_addr,memory_dev_size);
	if(originbuffer2) free(originbuffer2);
	close(ff);
	close(fp);
	return 0;
 }


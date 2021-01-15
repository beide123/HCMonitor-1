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

//#define _GNU_SOURCE

#define	 DMA
//#include "../GetCfgContent.h"
#include "../cfg_content.h"

#define phys_to_virtual(phys) (phys - DMA_CONTROL_REGISTER + memory_dev_addr)


char *memory_dev_addr;
extern int errno;

unsigned long long *ptr_writing_addr;
unsigned long long *ptr_stopt_addr;
unsigned long long  writing_addr;
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

int overflow = 0;

int fp;


//check the writing pointer to see whether overflow happens.

void *check_overflow()
{
	//maintain the writing_addr
	
	unsigned long long  last_writing_addr = DMA_BUFFER_ADDRESS;

	//glb_writing_addr = DMA_BUFFER_ADDRESS;

	gettimeofday(&tvpre_writing,&tz);

	//printf("tvpre_writing: tv_sec=%lld; tv_usec=%lld\n",tvpre_writing.tv_sec,tvpre_writing.tv_usec);
	//printf("tvpre_writing: %7.2f\n",(double)tvpre_writing.tv_sec + (double)tvpre_writing.tv_usec/1000000);
	
	writing_addr = *ptr_writing_addr;
	
	while(writing_addr)
	{
//printf("writing addr:%lx\n",writing_addr);
//printf("stopt_addr:%lx\n",stopt_addr);	
		if(writing_addr != last_writing_addr)
		{	//update the glb_writing_addr;
			writing_size = (writing_addr - last_writing_addr + DMA_BUFFER_TOTAL_SIZE) % DMA_BUFFER_TOTAL_SIZE;

			gettimeofday(&tvafter_writing,&tz);
			tv_writing = (double)(tvafter_writing.tv_sec - tvpre_writing.tv_sec) + (double)(tvafter_writing.tv_usec - tvpre_writing.tv_usec)/1000000;

			writing_speed = (double)writing_size/tv_writing/1024/1024;		//in MB/s
			printf("  $ The trace bandwidth is %7.2f MB/s;										\r",writing_speed);
		//	printf("  $ The trace bandwidth is %7.2f MB/s, writing_size is %7.2f MB;\r",writing_speed, (double)writing_size/1024/1024);
			fflush(stdout);

			last_writing_addr = writing_addr;
			glb_writing_addr += writing_size;

			gettimeofday(&tvpre_writing,&tz);
		}

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
		usleep(10000);//10ms,it will take at least 80ms to fillin a 64MB segment at the speed of 800MB/s
		writing_addr = *ptr_writing_addr;
	}
	
	stopt_addr = *ptr_stopt_addr;
	writing_size = (stopt_addr - last_writing_addr + DMA_BUFFER_TOTAL_SIZE) % DMA_BUFFER_TOTAL_SIZE;
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

	reading_addr = DMA_BUFFER_ADDRESS-1;
	//glb_reading_addr = DMA_BUFFER_ADDRESS-1;
	//maintain the reading_addr
	while(*ptr_writing_addr != 0)
	{
		if(*ptr_writing_addr == reading_addr + 1)
		{
			usleep(10000);
			continue;
		}
		
		else if(*ptr_writing_addr > reading_addr + 1)
		{
			tmp = *ptr_writing_addr;

			gettimeofday(&tvpre_reading,&tz);

			reading_size = *ptr_writing_addr-reading_addr-1;
			//if((((unsigned int)phys_to_virtual(reading_addr)+1) & 4095) || ((unsigned int)reading_size & 4095)){
			//if(((unsigned int)reading_size & 4095)){
			//if(1){
			//	printf("phys_to_virtual(reading_addr)+1 : %x, reading_size : %x\n", phys_to_virtual(reading_addr)+1, reading_size);
			//	fflush(stdout);
			//	exit(0);
			//}
			if((i = write( fp,phys_to_virtual(reading_addr)+1, reading_size)) != reading_size)
				printf("\n  #WARNINGing: write error!,%d,%d\n",i,errno);
			/*int rs1 = reading_size;
			while(rs1){
				if(((i = write( fp,((unsigned int)phys_to_virtual(reading_addr)+1) & ~4095, 16*1024))) != 16*1024)
					printf("\n  #WARNINGing: write error!,%d,%d\n",i,errno);
				rs1 -= 16*1024;
			}*/
			//printf("\nreading_size:%10.2f\n",(double)reading_size/1024/1024);
			//printf("glb_writing_addr:%10.2f,glb_reading_addr:%10.2f\n",(double)glb_writing_addr/1024/1024,(double)glb_reading_addr/1024/1024);

			gettimeofday(&tvafter_reading,&tz);

			tv_reading = (double)(tvafter_reading.tv_sec - tvpre_reading.tv_sec) + (double)(tvafter_reading.tv_usec - tvpre_reading.tv_usec)/1000000;
			reading_speed = (double)reading_size/tv_reading/1024/1024;
			printf("						the store speed is %7.2f MB/s.\r",reading_speed);
		//	printf("										the store speed is %7.2f MB/s, reading_size is %7.2f MB.\r",reading_speed, (double)reading_size/1024/1024);
			fflush(stdout);

			glb_reading_addr += reading_size;
		}
		
		else
		{
			tmp = DMA_BUFFER_ADDRESS;

			gettimeofday(&tvpre_reading,&tz);

			reading_size = DMA_BUFFER_ADDRESS+DMA_BUFFER_TOTAL_SIZE-reading_addr-1;
			/*if(((unsigned int)phys_to_virtual(reading_addr)+1) & 4095 || (unsigned int)reading_size & 4095){
				printf("phys_to_virtual(reading_addr)+1 : %x, reading_size : %x\n", phys_to_virtual(reading_addr)+1, reading_size);
				exit(0);
			}*/
			if(write( fp,phys_to_virtual(reading_addr)+1, reading_size) != reading_size)
				printf("\n  #WARNING: write error!\n");
			//printf("\nreading_size:%10.2f\n",(double)reading_size/1024/1024);
			//printf("glb_writing_addr:%10.2f,glb_reading_addr:%10.2f\n",(double)glb_writing_addr/1024/1024,(double)glb_reading_addr/1024/1024);

			gettimeofday(&tvafter_reading,&tz);

			tv_reading = (double)(tvafter_reading.tv_sec - tvpre_reading.tv_sec) + (double)(tvafter_reading.tv_usec - tvpre_reading.tv_usec)/1000000;
			reading_speed = (double)reading_size/tv_reading/1024/1024;
			printf("						the store speed is %7.2f MB/s.\r",reading_speed);
		//	printf("										the store speed is %7.2f MB/s, reading_size is %7.2f MB.\r",reading_speed, (double)reading_size/1024/1024);
			fflush(stdout);
			glb_reading_addr += reading_size;
			
			if(*ptr_writing_addr!=0 && *ptr_writing_addr!=DMA_BUFFER_ADDRESS)
			{
				tmp = *ptr_writing_addr;

				gettimeofday(&tvpre_reading,&tz);

				reading_size = *ptr_writing_addr-DMA_BUFFER_ADDRESS;
			/*if(((unsigned int)phys_to_virtual(reading_addr)+1) & 4095 || (unsigned int)reading_size & 4095){
				printf("phys_to_virtual(reading_addr)+1 : %x, reading_size : %x\n", phys_to_virtual(reading_addr)+1, reading_size);
				exit(0);
			}*/
				if(write( fp, phys_to_virtual(DMA_BUFFER_ADDRESS), reading_size) != reading_size)
					printf("\n  #WARNING: write error!\n");
				//printf("\nreading_size:%10.2f\n",(double)reading_size/1024/1024);
				//printf("glb_writing_addr:%10.2f,glb_reading_addr:%10.2f\n",(double)glb_writing_addr/1024/1024,(double)glb_reading_addr/1024/1024);

				gettimeofday(&tvafter_reading,&tz);

				tv_reading = (double)(tvafter_reading.tv_sec - tvpre_reading.tv_sec) + (double)(tvafter_reading.tv_usec - tvpre_reading.tv_usec)/1000000;
				reading_speed = (double)reading_size/tv_reading/1024/1024;
				printf("						the store speed is %7.2f MB/s.\r",reading_speed);
			//	printf("										the store speed is %7.2f MB/s, reading_size is %7.2f MB.\r",reading_speed, (double)reading_size/1024/1024);
				fflush(stdout);
				glb_reading_addr += reading_size;
			}
		}
	
		//printf("glb_reading_time: %7.2f, reading_addr:%x\n",glb_reading_time,tmp);	
		reading_addr = tmp - 1;

	}
		
	if(*ptr_stopt_addr > reading_addr+1)
	{
		reading_size = *ptr_stopt_addr-reading_addr-1;
		if(write( fp, phys_to_virtual(reading_addr)+1, reading_size) != reading_size)
			printf("\n  #WARNING: write error!\n");
		glb_reading_addr += reading_size;
		//printf("reading_size:%10.2f\n",(double)reading_size/1024/1024);
		//printf("\nglb_writing_addr:%10.2f,glb_reading_addr:%10.2f\n",(double)glb_writing_addr/1024/1024,(double)glb_reading_addr/1024/1024);
	}
	else if(*ptr_stopt_addr < reading_addr+1)
	{
		reading_size = DMA_BUFFER_ADDRESS+DMA_BUFFER_TOTAL_SIZE-reading_addr-1;
		if(write( fp, phys_to_virtual(reading_addr)+1, reading_size) != reading_size)
			printf("\n  #WARNING: write error!\n");
		glb_reading_addr += reading_size;
		//printf("reading_size:%10.2f\n",(double)reading_size/1024/1024);
		//printf("\nglb_writing_addr:%10.2f,glb_reading_addr:%10.2f\n",(double)glb_writing_addr/1024/1024,(double)glb_reading_addr/1024/1024);
		reading_size = *ptr_stopt_addr-DMA_BUFFER_ADDRESS;
		if(write( fp, phys_to_virtual(DMA_BUFFER_ADDRESS), reading_size) != reading_size)
			printf("\n  #WARNING: write error!\n");
		glb_reading_addr += reading_size;
		//printf("reading_size:%10.2f\n",(double)reading_size/1024/1024);
		//printf("\nglb_writing_addr:%10.2f,glb_reading_addr:%10.2f\n",(double)glb_writing_addr/1024/1024,(double)glb_reading_addr/1024/1024);
	}
}


int main(int argc, char **argv)
{
	char filename[64];
	
	if(argc != 2)
	{
		printf("Usage:./trace_collect filename\n");
		return 0;
	}
	memset(filename, 0, 64);
	strcpy(filename, argv[1]);
	strcat(filename, ".trace");
	//if((fp = open(filename,O_RDWR|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)) < 0)
	if((fp = open(filename,O_RDWR|O_CREAT|O_TRUNC|O_DIRECT,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)) < 0)
	{
		printf("Open file %s error\n", filename);
		return 0;
	}

	printf("page_size is %d\n", getpagesize());
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
			printf("unsigned long long : %7.2fMB/s\n",(double)memory_dev_size/tv_reading/1024/1024);
	
	/*
	*  Step.2 Read the traces from DMA buffer and store them into file
	*/	
	
	ptr_writing_addr = (unsigned long long *)memory_dev_addr;
	ptr_stopt_addr   = (unsigned long long *)(memory_dev_addr + 8);
	
	//set the writing_addr to zero at the beginning;
	*ptr_writing_addr = 0;
    
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
		printf("  %10.2fMB of trace collected.\n\n",(double)(glb_reading_addr-DMA_BUFFER_ADDRESS+1)/1024/1024);
	}
	else	
		printf("\n\n # ERROR: OVERFLOW HAPPENED!\n\n");
	//if(overflow) printf("  Overflow happened! %7.2f MB of trace lost!\n\n",(double)overflow*DMA_BUFFER_TOTAL_SIZE/1024/1024);
	printf("\n");
	munmap(memory_dev_addr,memory_dev_size);
	close(ff);
	close(fp);
	return 0;
 }


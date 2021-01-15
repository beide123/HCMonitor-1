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

#define	 DMA
#include "../GetCfgContent.h"
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

int main()
{
	int ff; //the file descriptor of the device memory_dev 
	unsigned long long memory_dev_size;	//the size of DMA buffer plus the DMA control registers
	
	ff = open("/dev/memory_dev", O_RDWR);
	if( ff < 0 ){
		printf("open /dev/memory_dev failed\n");
		return 0;
	}
		
	GetCfgContent();	//get the DMA information from ../cfg_content
	
	memory_dev_size = DMA_BUFFER_ADDRESS - DMA_CONTROL_REGISTER + DMA_BUFFER_TOTAL_SIZE;
	memory_dev_addr = mmap(0, memory_dev_size, PROT_READ|PROT_WRITE, MAP_SHARED,ff,0);
	/*
	*  Step.2 Read the traces from DMA buffer and store them into file
	*/	
	while(1)
	{	
		ptr_writing_addr = (unsigned long long *)memory_dev_addr;
		ptr_stopt_addr   = (unsigned long long *)(memory_dev_addr + 8);
		printf("%lld %lld\n", *ptr_writing_addr, *ptr_stopt_addr);
	}
}

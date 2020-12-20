#include <sys/mman.h>
#include <stddef.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define DMA
#define COMMAND
#define SAMPLE
//#define	DualChannel
//#define UP4G

#include "offset.h"
//#include "../GetCfgContent.h"
#include "../cfg_content.h"



int main()
{

    int ff;
    if( (ff = open("/dev/hmtt_pcie", O_RDWR))<0 )
    {
        printf("Open /dev/hmtt_pcie failed!\n");
        return 0;
    }

    char *fpga_ram_start;
    fpga_ram_start = mmap(NULL,fpga_ram_size,PROT_READ|PROT_WRITE,MAP_SHARED,ff,0);
    if((long long)fpga_ram_start<0)
    {
        printf("Memory map failed!\n");
        return -1;
    }


    *(volatile unsigned long*)(fpga_ram_start + 0x4000ULL)   = 0x02ULL;

    
    printf("Stop done!\n");
    
    close(ff);

}

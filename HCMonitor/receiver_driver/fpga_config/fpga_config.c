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

    int offset = 6;

    /*printf("cfg_start:%lx,end:%lx\n",cfg_start,cfg_end);
    printf("sample_start1:%lx,end1:%lx\n",sample_start1,sample_end1);
    printf("sample_start2:%lx,end2:%lx\n",sample_start2,sample_end2);
    printf("sample_start3:%lx,end3:%lx\n",sample_start3,sample_end3);
    printf("sample_start4:%lx,end4:%lx\n",sample_start4,sample_end4);*/
    //configue the DMA registers
    *(volatile unsigned long*)(fpga_ram_start + DMA_BUF_ADDR_ADDR)	=	DMA_BUF_ADDR;
    *(volatile unsigned long*)(fpga_ram_start + DMA_BUF_SIZE_ADDR)	=	DMA_BUF_SIZE;
    *(volatile unsigned long*)(fpga_ram_start + DMA_REG_ADDR_ADDR)	=	DMA_REG_ADDR;
    *(volatile unsigned long*)(fpga_ram_start + DMA_SEG_SIZE_ADDR)	=	DMA_SEG_SIZE;
   
    //configue the Sample registers
    *(volatile unsigned long*)(fpga_ram_start + PA_MAP_1_2_ADDR)	=	PA_MAP_1_2;
    *(volatile unsigned long*)(fpga_ram_start + PA_MAP_3_4_ADDR)	=	PA_MAP_3_4;
    *(volatile unsigned long*)(fpga_ram_start + PA_MAP_5_6_ADDR)	=	PA_MAP_5_6;
    *(volatile unsigned long*)(fpga_ram_start + PA_MAP_7_8_ADDR)	=	PA_MAP_7_8;
    *(volatile unsigned long*)(fpga_ram_start + PA_MAP_9_10_ADDR)	=	PA_MAP_9_10;
    *(volatile unsigned long*)(fpga_ram_start + PA_MAP_11_12_ADDR)	=	PA_MAP_11_12;
    *(volatile unsigned long*)(fpga_ram_start + PA_MAP_13_14_ADDR)	=	PA_MAP_13_14;
    *(volatile unsigned long*)(fpga_ram_start + PA_MAP_15_16_ADDR)	=	PA_MAP_15_16;
    *(volatile unsigned long*)(fpga_ram_start + PA_MAP_17_18_ADDR)	=	PA_MAP_17_18;
    *(volatile unsigned long*)(fpga_ram_start + PA_MAP_19_20_ADDR)	=	PA_MAP_19_20;
    *(volatile unsigned long*)(fpga_ram_start + PA_MAP_21_22_ADDR)	=	PA_MAP_21_22;
    *(volatile unsigned long*)(fpga_ram_start + PA_MAP_23_24_ADDR)	=	PA_MAP_23_24;
    *(volatile unsigned long*)(fpga_ram_start + PA_MAP_25_26_ADDR)	=	PA_MAP_25_26;
    *(volatile unsigned long*)(fpga_ram_start + PA_MAP_27_28_ADDR)	=	PA_MAP_27_28;
    *(volatile unsigned long*)(fpga_ram_start + PA_MAP_29_30_ADDR)	=	PA_MAP_29_30;
    *(volatile unsigned long*)(fpga_ram_start + PA_MAP_31_32_ADDR)	=	PA_MAP_31_32;
    *(volatile unsigned long*)(fpga_ram_start + PA_MAP_33_34_ADDR)	=	PA_MAP_33_34;
    *(volatile unsigned long*)(fpga_ram_start + PA_MAP_35_36_ADDR)	=	PA_MAP_35_36;

    *(volatile unsigned long*)(fpga_ram_start + PA_TRACE_CFG_ADDR)	=	PA_TRACE_CFG;

    *(volatile unsigned long*)(fpga_ram_start + PA_RANGE_START_ADDR)	=	PA_RANGE_START >> offset;
    *(volatile unsigned long*)(fpga_ram_start + PA_RANGE_END_ADDR)	=	PA_RANGE_END >> offset;

    //configue the Config_space registers
    *(volatile unsigned long*)(fpga_ram_start + CFG_START_ADDR)		=	CFG_START >> offset;
    *(volatile unsigned long*)(fpga_ram_start + CFG_END_ADDR)		=	CFG_END >> offset;


    *(volatile unsigned long*)(fpga_ram_start + PA_TRACE_CTRL_ADDR)	=	0x0000000000000000ULL;

    
    printf("Configue done!\n");
    
    close(ff);

}

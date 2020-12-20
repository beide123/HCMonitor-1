/************memory_dev.c**************/
#include <linux/kernel.h>
#include <linux/errno.h>
//#include <linux/config.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/mm.h>
//#include <linux/wrapper.h> /* for mem_map_(un)reserve */
#include <asm/io.h> /* for virt_to_phys */
#include <linux/slab.h> /* for kmalloc and kfree */

//#include "dma_buf.h"
#include "../cfg_content.h"


/****************************************************************/
//clc added: lack the following two header files, error in kernel 2.6.32
/************************************************************/
#include <linux/fs.h>
#include <linux/cdev.h>

#ifndef __KERNEL__ 
#define __KERNEL__ 
#endif
/******************************************************************************/
//end of clc added
/****************************************************************************/

MODULE_LICENSE("GPL");
MODULE_AUTHOR("LL");
MODULE_DESCRIPTION("MTT test");


//MODULE_PARM(mem_start, "i");
//MODULE_PARM(mem_size, "i");
//module_param(mem_start, int, 0644); 
//module_param(mem_size, int, 0644); 
/*
module_param(myshort, short, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

MODULE_PARM_DESC(myshort, "A short integer");

module_param(myint, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

MODULE_PARM_DESC(myint, "An integer");

module_param(mylong, long, S_IRUSR);

MODULE_PARM_DESC(mylong, "A long integer");

module_param(mystring, charp, 0000);

MODULE_PARM_DESC(mystring, "A character string");
*/
/*struct memory_dev_dev{
	struct semaphore sem; // mutual exclusion semaphore 
	struct cdev cdev; // Char device structure 
};*/

//the start address of the buffer is DMA_CONTROL_REGISTER, the size of the buffer is (DMA_BUFFER_ADDRESS-DMA_CONTROL_REGISTER+DMA_BUFFER_TOTAL_SIZE)B
static unsigned long long mem_start = DMA_REG_ADDR, mem_size = DMA_BUF_ADDR - DMA_REG_ADDR + DMA_BUF_SIZE;


static char *reserve_virt_addr;
static int major = 0;
module_param(major, int, 0);

static dev_t dev_memory_dev;
static struct cdev memory_dev_cdev;

static int mmapdrv_init(void);
static void  mmapdrv_clean(void); 
int mmapdrv_open(struct inode *inode, struct file *file);
int mmapdrv_release(struct inode *inode, struct file *file);
int mmapdrv_mmap(struct file *file, struct vm_area_struct *vma);

static struct file_operations mmapdrv_fops =
{
	.owner = THIS_MODULE,
	.mmap = mmapdrv_mmap,
	.open = mmapdrv_open,
	.release = mmapdrv_release
//	ioctl:		mmapdrv_ioctl,
};

/*
#ifdef MODULE
EXPORT_SYMBOL(mmapdrv_init);
EXPORT_SYMBOL(mmapdrv_clean);
EXPORT_SYMBOL(mmapdrv_open);
EXPORT_SYMBOL(mmapdrv_release);
EXPORT_SYMBOL(mmapdrv_mmap);
#endif
*/


static int __init mmapdrv_init(void)
{
	
	int result;
	int err;
	if( major ){
		dev_memory_dev = MKDEV( major,0 );//first dev
		result = register_chrdev_region( dev_memory_dev, 1, "memory_dev" );
	}
	else{
		result = alloc_chrdev_region( &dev_memory_dev, 0, 1, "memory_dev" );
		major = MAJOR( dev_memory_dev );
	}
	if( result < 0 )
	{
		printk("<0>memory_dev: unable to get major %d\n", major);
		return result;
	}
	if( major == 0 )
	{
		major = result;
	}
	cdev_init( &memory_dev_cdev, &mmapdrv_fops);
	memory_dev_cdev.owner = THIS_MODULE;
	memory_dev_cdev.ops = &mmapdrv_fops;
	err = cdev_add ( &memory_dev_cdev, dev_memory_dev, 1);
	/* Fail gracefully if need be */
	if (err)
	{
		printk ("<0>Error %d adding memory_dev cdev--%d:%d", err, major,MINOR(dev_memory_dev) );
	}

	printk("<0>memory_dev device major = %d\n", major);

	//reserve_virt_addr = ioremap(mem_start , mem_size );
/*	reserve_virt_addr = ioremap_cache(mem_start , mem_size );
	printk("<0>reserve_virt_addr = 0x%lx\n", (unsigned long)reserve_virt_addr);
	if (reserve_virt_addr==NULL)
	{
		printk("<0>error happened in memory_dev\n");
		//unregister_chrdev(major, "mmapdrv");
		cdev_del( &memory_dev_cdev );
        	unregister_chrdev_region( dev_memory_dev, 1 );
		return - ENODEV;
	}

	memset(reserve_virt_addr,3,mem_size);

	int i;
	for(i = 0; i < mem_size; i++){
		if(*((char *)reserve_virt_addr + i) != 3){
			printk("something wrong %d\n", i);
			break;
		}
	}
*/
	printk("done \n");
	return 0;
}

/* remove the module */
static void __exit mmapdrv_clean(void)
{
	if (reserve_virt_addr)
	iounmap(reserve_virt_addr);
	
	cdev_del( &memory_dev_cdev );
	unregister_chrdev_region( dev_memory_dev, 1 );
	//unregister_chrdev(major, "mmapdrv");
	printk("<0>memory_dev module exit\n");
	return ;
}
module_init(mmapdrv_init);
module_exit(mmapdrv_clean);
int mmapdrv_open(struct inode *inode, struct file *file)
{
//	MOD_INC_USE_COUNT;
	return (0);
}

int mmapdrv_release(struct inode *inode, struct file *file)
{
//	MOD_DEC_USE_COUNT;
	return (0);
}

int mmapdrv_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
	unsigned long size = vma->vm_end - vma->vm_start;
	unsigned long prot = pgprot_val(vma->vm_page_prot);
//	prot |= (_PAGE_PCD | _PAGE_PWT | _PAGE_PROTNONE); 
//	prot |= (_PAGE_PWT | _PAGE_PROTNONE); 
	prot &= ~_PAGE_PCD;
//	prot |= _PAGE_PWT; 
	printk("<1>prot=%lx\n",prot);
	//if (size > mem_size )
	if (0)
	{
		printk("<0>size too big\n");
		return ( - ENXIO);
	}
	printk("<1>offset = %lx,size=%lx\n",offset,size);
	offset = offset + mem_start ;

/* we do not want to have this area swapped out, lock it */
	vma->vm_flags |= VM_LOCKED;
 	vma->vm_flags |= VM_IO;
//	vma->vm_flags |= VM_RESERVED;
	vma->vm_flags |= VM_READ;
	vma->vm_flags |= VM_WRITE;
	vma->vm_flags |= VM_MAYREAD;
	vma->vm_flags |= VM_MAYWRITE;
	vma->vm_flags |= VM_DONTEXPAND;
	vma->vm_flags |= VM_GROWSUP;
	vma->vm_flags |= VM_SHARED;
	if (remap_pfn_range(vma, vma->vm_start, offset>>PAGE_SHIFT, size,__pgprot(prot)))
	//if (remap_pfn_range_sp(vma, vma->vm_start, offset>>PAGE_SHIFT, size,__pgprot(prot)))
	{
		printk("<0>remap page range failed\n");
		return - ENXIO;
	}
	printk("<1>keep mem mmaped\n");
	return (0);
}


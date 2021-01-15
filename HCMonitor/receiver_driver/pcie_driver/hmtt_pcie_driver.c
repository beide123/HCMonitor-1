/*HMTT v3.0 PCIE驱动程序*/

//#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#define HMTT_VendorID	(0x10ee)
#define HMTT_DeviceID	(0x8024)
#define BAR_0			0


static char *MemVirtAddr;
unsigned long mem_size;
unsigned long mem_start;

static int major =0;
module_param(major,int,0);

static dev_t HMTT_pcie_dev;
static struct cdev HMTT_pcie_cdev;

static int probe(struct pci_dev *dev, const struct pci_device_id *id)
{
	/* Do probing type stuff here.  
	 * Like calling request_region();
	 */
	printk("enter probe\n");
	pci_enable_device(dev);

	//Make sure it is enabled as a bus mastering device
   	printk("begin pci set master\n");
    pci_set_master(dev);
   	printk("end pci set master\n");

	return 0;
}

static void remove(struct pci_dev *dev)
{
	/* clean up any allocated resources and stuff here.
	 * like call release_region();
	 */
}

/*该驱动程序所能操作的设备id列表*/
static struct pci_device_id ids[] = {
    {HMTT_VendorID, HMTT_DeviceID, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
    {0,}
};
MODULE_DEVICE_TABLE(pci, ids);

static struct pci_driver HMTT_pcie_driver = {
	.name = "hmtt_pcie",
	.id_table = ids,
	.probe = probe,
	.remove = remove,
};



int HMTT_pcie_open(struct inode *inode, struct file *filp)
{
	//try_get_module(THIS_MODULE);
	printk("open succeed\n");
	return 0;
}

int HMTT_pcie_mmap(struct file *file, struct vm_area_struct *vma)
{
     unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
     unsigned long size = vma->vm_end - vma->vm_start;
     unsigned long prot = pgprot_val(vma->vm_page_prot);
 //  prot |= (_PAGE_PCD | _PAGE_PWT | _PAGE_PROTNONE); 
     prot |= (_PAGE_PWT | _PAGE_PROTNONE); 
     printk("prot=%lx\n",prot);
     if (size > mem_size )
     {
         printk("<0>size too big\n");
         return ( - ENXIO);
     }
     printk("offset = %lx,size=%lx\n",offset,size);
     offset = offset + mem_start ;
 
/* we do not want to have this area swapped out, lock it */
     vma->vm_flags |= VM_LOCKED;
     vma->vm_flags |= VM_IO;
//     vma->vm_flags |= VM_RESERVED;
     vma->vm_flags |= VM_READ;
     vma->vm_flags |= VM_WRITE;
     vma->vm_flags |= VM_MAYREAD;
     vma->vm_flags |= VM_MAYWRITE;
     vma->vm_flags |= VM_DONTEXPAND;
     vma->vm_flags |= VM_GROWSUP;
     vma->vm_flags |= VM_SHARED;
     if (remap_pfn_range(vma, vma->vm_start, offset>>PAGE_SHIFT, size,__pgprot(prot)))
     {
         printk("<0>remap page range failed\n");
         return - ENXIO;
     }
     printk("keep mem mmaped\n");
     return (0);
}

int HMTT_pcie_release(struct inode *inode, struct file *filp)
{
	//module_put(THIS_MODULE);	
	printk("release succeed\n");
	return 0;
}

static struct file_operations HMTT_pcie_fops =
{
	.owner = THIS_MODULE,
	.open = HMTT_pcie_open,
    .mmap = HMTT_pcie_mmap,
	.release = HMTT_pcie_release
};

static int HMTT_pcie_init(void);
//static void HMTT_pcie_clean(void);

#ifdef MODULE
EXPORT_SYMBOL(HMTT_pcie_init);
//EXPORT_SYMBOL(HMTT_pcie_clean);
EXPORT_SYMBOL(HMTT_pcie_open);
EXPORT_SYMBOL(HMTT_pcie_release);
EXPORT_SYMBOL(HMTT_pcie_mmap);
#endif 


static int __init HMTT_pcie_init(void)
{
	
	if(!pci_register_driver(&HMTT_pcie_driver)) //pci_register_driver函数返回0表示注册成功
		printk("init:register succeed\n");
	else 
		printk("<0>init:register failed\n");

    int result;
    int err;
    if( major ){
         HMTT_pcie_dev = MKDEV( major,0 );//first dev
         result = register_chrdev_region( HMTT_pcie_dev, 1, "HMTT_pcie" );
         printk("if:%d\n",major);
    }
    else{
         result = alloc_chrdev_region( &HMTT_pcie_dev, 0, 1, "HMTT_pcie_cdev" );
         major = MAJOR( HMTT_pcie_dev );
         printk("else:%d\n",major);
    }
    if( result < 0 )
    {
         printk("<0>HMTT_pcie: unable to get major %d\n", major);
         return result;
    }
    if( major == 0 )
    {
         major = result; 
    }
    cdev_init( &HMTT_pcie_cdev, &HMTT_pcie_fops);
    HMTT_pcie_cdev.owner = THIS_MODULE;
    HMTT_pcie_cdev.ops = &HMTT_pcie_fops;
    err = cdev_add ( &HMTT_pcie_cdev, HMTT_pcie_dev, 1);
    /* Fail gracefully if need be */
    if (err)
    {
         printk ("<0>Error %d adding HMTT_pcie cdev--%d:%d", err, major,MINOR(HMTT_pcie_dev) );
    }

	struct pci_dev *dev = pci_get_device(HMTT_VendorID, HMTT_DeviceID, NULL);

    if(pci_resource_start(dev, BAR_0)==0)
    {
        printk("<0>BAR failed\n");
        return 0;
    }
    else
    {
        printk("BAR=%lx\n",pci_resource_start(dev, BAR_0));
    }
	mem_start= pci_resource_start(dev,BAR_0);
    mem_size = pci_resource_len(dev,BAR_0);
    printk("mem_start:%llx,mem_size:%llx\n",mem_start,mem_size);
    MemVirtAddr = ioremap( mem_start, mem_size );
    if(MemVirtAddr==NULL)
	{
		printk("<0>ioremap failed\n");
		return 0;
	}
	
	return 0;

}
static void __exit HMTT_pcie_exit(void)
{
    printk("remove %d\n",major);
    unregister_chrdev_region(HMTT_pcie_dev,1);
    cdev_del(&HMTT_pcie_cdev);
	pci_unregister_driver(&HMTT_pcie_driver);
	iounmap(MemVirtAddr);
	printk("exit driver\n");
}

MODULE_LICENSE("GPL");

module_init(HMTT_pcie_init);
module_exit(HMTT_pcie_exit);

#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xaa3ca794, "module_layout" },
	{ 0x7ffbe1b2, "cdev_del" },
	{ 0x12d96d22, "cdev_init" },
	{ 0x8ba7df31, "param_ops_int" },
	{ 0x3fd78f3b, "register_chrdev_region" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0xe8fc855d, "pci_set_master" },
	{ 0x27e1a049, "printk" },
	{ 0xdf893afb, "cdev_add" },
	{ 0x93a219c, "ioremap_nocache" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x554a772, "pci_unregister_driver" },
	{ 0xcd6480e8, "remap_pfn_range" },
	{ 0xedc03953, "iounmap" },
	{ 0xf2dc3df6, "__pci_register_driver" },
	{ 0xee0ee769, "pci_get_device" },
	{ 0x8b8ffec1, "pci_enable_device" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("pci:v000010EEd00008024sv*sd*bc*sc*i*");

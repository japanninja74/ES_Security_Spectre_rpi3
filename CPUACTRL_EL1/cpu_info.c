#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/smp.h>

#define DRVR_NAME "cpu_info"

static int __init
init(void)
{
	long int r,f;
	printk(KERN_INFO "[" DRVR_NAME "] initialized");
	// MMU
	asm volatile("mrs %0, S3_0_C1_C0_0" : "=r" (r));
	printk(KERN_INFO "[" DRVR_NAME "] MMU STATUS: %s",(r&0x1)? "enabled":"disabled");
	printk(KERN_INFO "[" DRVR_NAME "] L1 DATA AND UNIFIED CACHE: %s",(r&0x3)? "enabled":"disabled");

	// Prefetcher
	asm volatile ("mrs %0, S3_1_C15_C2_0" : "=r" (f));
	printk(KERN_INFO "[" DRVR_NAME "] PREFETCHER STATUS: %ld",f);
    return 0;
}

static void __exit
fini(void)
{
	printk(KERN_INFO "[" DRVR_NAME "] unloaded");
}

MODULE_AUTHOR("");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_DESCRIPTION("");
MODULE_VERSION("0.0.1");
module_init(init);
module_exit(fini);

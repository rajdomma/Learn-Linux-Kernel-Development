/*
 * ch8/slab1/slab1.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Learn Linux Kernel Development"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Learn-Linux-Kernel-Development
 *
 * From: Ch 8 : Kernel Memory Allocation for Module Authors, Part 1
 ****************************************************************
 * Brief Description:
 *
 * For details, please refer the book, Ch 8.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>

#define OURMODNAME   "slab1"

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LLKD book:ch8/slab1: k[m|z]alloc, kfree, basic demo");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

static char *gkptr;
struct myctx {
	u32 iarr[100];
	u64 uarr[100];
	char uname[128], passwd[16], config[16];
};
static struct myctx *ctx;

static int __init slab1_init(void)
{
	/* 1. Allocate slab memory for 1 KB using the kmalloc() */
	gkptr = kmalloc(1024, GFP_KERNEL);
	if (!gkptr) {
		pr_warn("%s: kmalloc failed!\n", OURMODNAME);
		/* As mentioned earlier, there is really no need to print an
		 * error msg when a memory alloc fails; the situation
		 * "shouldn't" typically occur, and if it does, the kernel will
		 * emit a chain of messages in any case. Here, we do so as it's
		 * a 'learning' program..
		 */
		goto out_fail1;
	}
	print_hex_dump_bytes("gkptr before memset: ", DUMP_PREFIX_OFFSET, gkptr, 32);
	memset(gkptr, 'm', 1024);
	print_hex_dump_bytes(" gkptr after memset: ", DUMP_PREFIX_OFFSET, gkptr, 32);

	/* 2. Allocate memory for and initialize our 'context' structure */
	ctx = kzalloc(sizeof(struct myctx), GFP_KERNEL);
	if (!ctx) {
		pr_warn("%s: kzalloc failed!\n", OURMODNAME);
		goto out_fail2;
	}
	pr_info("%s: context struct alloc'ed and initialized\n", OURMODNAME);
	print_hex_dump_bytes("ctx: ", DUMP_PREFIX_OFFSET, ctx, 32);

	return 0;		/* success */

out_fail2:
	kfree(gkptr);
out_fail1:
	return -ENOMEM;
}

static void __exit slab1_exit(void)
{
	kfree(ctx);
	kfree(gkptr);
	pr_info("%s: freed slab memory, removed\n", OURMODNAME);
}

module_init(slab1_init);
module_exit(slab1_exit);

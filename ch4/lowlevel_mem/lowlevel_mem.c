/*
 * ch4/lowlevel_mem/lowlevel_mem.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Development Cookbook"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Development-Cookbook
 *
 * From: Ch 4: Kernel Memory Allocation for Module Authors
 ****************************************************************
 * Brief Description:
 * A quick demo of the essential 'low-level' / page allocator / Buddy System
 * Allocator (BSA) APIs for allocating and freeing memory chunks in kernel
 * space.
 *
 * For details, please refer the book, Ch 4.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mm.h>

#define OURMODNAME    "lowlevel_mem"

MODULE_DESCRIPTION("Demo kernel module to exercise essential page allocator APIs.");
MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_LICENSE("MIT");

static const void *gptr1, *gptr2, *gptr3, *gptr4, *gptr5;
static int bsa_alloc_order = 5;
module_param_named(order, bsa_alloc_order, int, 0660);
MODULE_PARM_DESC(order, "Order of the allocation (power-to-raise-2-to)");

/*
 * powerof(base, exponent)
 * Simple 'library' function to calculate and return
 *  @base to-the-power-of @exponent
 * f.e. powerof(2, 5) returns 2^5 = 32.
 * Returns -1UL on failure.
 */
static u64 powerof(int base, int exponent)
{
	u64 res = 1;

	if (base == 0)		// 0^e = 0
		return 0;
	if (base <= 0 || exponent < 0)
		return -1UL;
	if (exponent == 0)	// b^0 = 1
		return 1;
	while (exponent--)
		res *= base;
	return res;
}
EXPORT_SYMBOL(powerof);

/*
 * bsa_alloc : test some of the bsa (buddy system allocator
 * aka page allocator) APIs
 */
static int bsa_alloc(void)
{
	int stat = -ENOMEM;
	u64 numpg2alloc = 0;
	const struct page *pg_ptr1;

	/* 1. Allocate one page with the __get_free_page() API */
	gptr1 = (void *) __get_free_page(GFP_KERNEL);
	if (!gptr1) {
		pr_warn("%s: __get_free_page() failed!\n", OURMODNAME);
		goto out1;
	}
	pr_info("%s: 1. __get_free_page() alloc'ed 1 page from the BSA @ %pK\n",
		OURMODNAME, gptr1);

	/* 2. Allocate 2^bsa_alloc_order pages with the __get_free_pages() API */
	numpg2alloc = powerof(2, bsa_alloc_order);
	gptr2 = (void *) __get_free_pages(GFP_KERNEL, bsa_alloc_order);
	if (!gptr2) {
		pr_warn("%s: __get_free_pages() failed!\n", OURMODNAME);
		goto out2;
	}
	pr_info("%s: 2. __get_free_pages() alloc'ed 2^%d = %lld page(s) = %lld bytes\n"
		" from the BSA @ %pK\n",
		OURMODNAME, bsa_alloc_order, powerof(2, bsa_alloc_order),
		numpg2alloc * PAGE_SIZE, gptr2);
	pr_info(" (PAGE_SIZE = %ld bytes)\n", PAGE_SIZE);

	/* 3. Allocate and init one page with the get_zeroed_page() API */
	gptr3 = (void *) get_zeroed_page(GFP_KERNEL);
	if (!gptr3) {
		pr_warn("%s: get_zeroed_page() failed!\n", OURMODNAME);
		goto out3;
	}
	pr_info("%s: 3. get_zeroed_page() alloc'ed 1 page from the BSA @ %pK\n",
		OURMODNAME, gptr3);

	/* 4. Allocate and init one page with the alloc_page() API.
	 * Careful! It does not return the alloc'ed page ptr but rather the ptr
	 * to the metadata structure 'page' representing the allocated page:
	 *    struct page * alloc_page(gfp_mask);
	 * So, we use the page_address() helper to convert it to a kernel
	 * logical (or virtual) address.
	 */
	pg_ptr1 = alloc_page(GFP_KERNEL);
	if (!pg_ptr1) {
		pr_warn("%s: alloc_page() failed!\n", OURMODNAME);
		goto out4;
	}
	gptr4 = page_address(pg_ptr1);
	pr_info("%s: 4. alloc_page() alloc'ed 1 page from the BSA @ %pK\n"
		" (page addr=%pK\n)",
		OURMODNAME, (void *)gptr4, pg_ptr1);

	/* 5. Allocate and init 2^3 = 8 pages with the alloc_pages() API.
	 * < Same warning as above applies here too! >
	 */
	gptr5 = page_address(alloc_pages(GFP_KERNEL, 3));
	if (!gptr5) {
		pr_warn("%s: alloc_pages() failed!\n", OURMODNAME);
		goto out5;
	}
	pr_info("%s: 5. alloc_pages() alloc'ed %lld pages from the BSA @ %pK\n",
		OURMODNAME, powerof(2, 3), (void *)gptr5);

	return 0;
out5:
	free_page((unsigned long) gptr4);
out4:
	free_page((unsigned long) gptr3);
out3:
	free_pages((unsigned long) gptr2, bsa_alloc_order);
out2:
	free_page((unsigned long) gptr1);
out1:
	return stat;
}

static int __init lowlevel_mem_init(void)
{
	return bsa_alloc();
}

static void __exit lowlevel_mem_exit(void)
{
	pr_info("%s: free-ing up the BSA memory chunks...\n", OURMODNAME);
	free_page((unsigned long) gptr1);
	free_pages((unsigned long) gptr2, bsa_alloc_order);
	free_page((unsigned long) gptr3);
	free_page((unsigned long) gptr4);
	free_pages((unsigned long) gptr5, 3);
	pr_info("%s: removed\n", OURMODNAME);
}

module_init(lowlevel_mem_init);
module_exit(lowlevel_mem_exit);
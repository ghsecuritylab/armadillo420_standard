/*
 *  Copyright 2004-2007 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __ASM_ARCH_MXC_IO_H__
#define __ASM_ARCH_MXC_IO_H__

/* Allow IO space to be anywhere in the memory */
#define IO_SPACE_LIMIT 0xffffffff

/* io address mapping macro */
#define __io(a)			((void __iomem *)(a))

#define __mem_pci(a)		(a)
#define __mem_isa(a)		(a)

/*!
 * Validate the pci memory address for ioremap.
 */
#define iomem_valid_addr(iomem,size)	(1)

/*!
 * Convert PCI memory space to a CPU physical address
 */
#define iomem_to_phys(iomem)	(iomem)

extern void __iomem *__mxc_ioremap(unsigned long cookie, size_t size,
				   unsigned int mtype);
extern void __mxc_iounmap(void __iomem *addr);

#define __arch_ioremap(a, s, f) __mxc_ioremap(a, s, f)
#define __arch_iounmap(a)	 __mxc_iounmap(a)

#endif

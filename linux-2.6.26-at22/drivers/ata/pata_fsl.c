/*
 * Freescale integrated PATA driver
 */

/*
 * Copyright 2007 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/blkdev.h>
#include <scsi/scsi_host.h>
#include <linux/ata.h>
#include <linux/libata.h>
#include <linux/platform_device.h>
#include <linux/fsl_devices.h>
#include <asm/dma.h>

#define DRV_NAME "pata_fsl"
#define DRV_VERSION "1.0"

struct pata_fsl_priv {
	int ultra;
	u8 *fsl_ata_regs;
	int dma_rchan;
	int dma_wchan;
	int dma_done;
	int dma_dir;
};

enum {
	/* various constants */

	FSL_ATA_MAX_SG_LEN      = 65534,

	/* offsets to registers */

	FSL_ATA_TIMING_REGS     = 0x00,
	FSL_ATA_FIFO_FILL       = 0x20,
	FSL_ATA_CONTROL         = 0x24,
	FSL_ATA_INT_PEND        = 0x28,
	FSL_ATA_INT_EN          = 0x2C,
	FSL_ATA_INT_CLEAR       = 0x30,
	FSL_ATA_FIFO_ALARM      = 0x34,
	FSL_ATA_DRIVE_DATA      = 0xA0,
	FSL_ATA_DRIVE_CONTROL   = 0xD8,

	/* bits within FSL_ATA_CONTROL */

	FSL_ATA_CTRL_FIFO_RST_B    = 0x80,
	FSL_ATA_CTRL_ATA_RST_B     = 0x40,
	FSL_ATA_CTRL_FIFO_TX_EN    = 0x20,
	FSL_ATA_CTRL_FIFO_RCV_EN   = 0x10,
	FSL_ATA_CTRL_DMA_PENDING   = 0x08,
	FSL_ATA_CTRL_DMA_ULTRA     = 0x04,
	FSL_ATA_CTRL_DMA_WRITE     = 0x02,
	FSL_ATA_CTRL_IORDY_EN      = 0x01,

	/* bits within the interrupt control registers */

	FSL_ATA_INTR_ATA_INTRQ1      = 0x80,
	FSL_ATA_INTR_FIFO_UNDERFLOW  = 0x40,
	FSL_ATA_INTR_FIFO_OVERFLOW   = 0x20,
	FSL_ATA_INTR_CTRL_IDLE       = 0x10,
	FSL_ATA_INTR_ATA_INTRQ2      = 0x08,
};

/*
 * This structure contains the timing parameters for
 * ATA bus timing in the 5 PIO modes.  The timings
 * are in nanoseconds, and are converted to clock
 * cycles before being stored in the ATA controller
 * timing registers.
 */
static struct {
	short t0, t1, t2_8, t2_16, t2i, t4, t9, tA;
} pio_specs[] = {
	[0] = {
		.t0 = 600, .t1 = 70, .t2_8 = 290, .t2_16 = 165, .t2i = 0,
		.t4 = 30,.t9 = 20,.tA = 50
	},
	[1] = {
		.t0 = 383, .t1 = 50, .t2_8 = 290, .t2_16 = 125, .t2i = 0,
		.t4 = 20, .t9 = 15, .tA = 50
	},
	[2] = {
		.t0 = 240, .t1 = 30, .t2_8 = 290, .t2_16 = 100, .t2i = 0,
		.t4 = 15, .t9 = 10, .tA = 50
	},
	[3] = {
		.t0 = 180, .t1 = 30, .t2_8 = 80, .t2_16 = 80, .t2i = 0,
		.t4 = 10, .t9 = 10, .tA = 50
	},
	[4] = {
		.t0 = 120, .t1 = 25, .t2_8 = 70, .t2_16 = 70, .t2i = 0,
		.t4 = 10, .t9 = 10, .tA = 50
	},
};

#define NR_PIO_SPECS (sizeof pio_specs / sizeof pio_specs[0])

/*
 * This structure contains the timing parameters for
 * ATA bus timing in the 3 MDMA modes.  The timings
 * are in nanoseconds, and are converted to clock
 * cycles before being stored in the ATA controller
 * timing registers.
 */
static struct {
	short t0M, tD, tH, tJ, tKW, tM, tN, tJNH;
} mdma_specs[] = {
	[0] = {
		.t0M = 480, .tD = 215, .tH = 20, .tJ = 20, .tKW = 215,
		.tM = 50, .tN = 15, .tJNH = 20
	},
	[1] = {
		.t0M = 150, .tD = 80, .tH = 15, .tJ = 5, .tKW = 50,
		.tM = 30, .tN = 10, .tJNH = 15
	},
	[2] = {
		.t0M = 120, .tD = 70, .tH = 10, .tJ = 5, .tKW = 25,
		.tM = 25, .tN = 10, .tJNH = 10
	},
};

#define NR_MDMA_SPECS (sizeof mdma_specs / sizeof mdma_specs[0])

/*
 * This structure contains the timing parameters for
 * ATA bus timing in the 6 UDMA modes.  The timings
 * are in nanoseconds, and are converted to clock
 * cycles before being stored in the ATA controller
 * timing registers.
 */
static struct {
	short t2CYC, tCYC, tDS, tDH, tDVS, tDVH, tCVS, tCVH, tFS_min, tLI_max,
	    tMLI, tAZ, tZAH, tENV_min, tSR, tRFS, tRP, tACK, tSS, tDZFS;
} udma_specs[] = {
	[0] = {
		.t2CYC = 235, .tCYC = 114, .tDS = 15, .tDH = 5, .tDVS = 70,
		.tDVH = 6, .tCVS = 70, .tCVH = 6, .tFS_min = 0,
		.tLI_max = 100, .tMLI = 20, .tAZ = 10, .tZAH = 20,
		.tENV_min = 20, .tSR = 50, .tRFS = 75, .tRP = 160,
		.tACK = 20, .tSS = 50, .tDZFS = 80
	},
	[1] = {
		.t2CYC = 156, .tCYC = 75, .tDS = 10, .tDH = 5, .tDVS = 48,
		.tDVH = 6, .tCVS = 48, .tCVH = 6, .tFS_min = 0,
		.tLI_max = 100, .tMLI = 20, .tAZ = 10, .tZAH = 20,
		.tENV_min = 20, .tSR = 30, .tRFS = 70, .tRP = 125,
		.tACK = 20, .tSS = 50, .tDZFS = 63
	},
	[2] = {
		.t2CYC = 117, .tCYC = 55, .tDS = 7, .tDH = 5, .tDVS = 34,
		.tDVH = 6, .tCVS = 34, .tCVH = 6, .tFS_min = 0,
		.tLI_max = 100, .tMLI = 20, .tAZ = 10, .tZAH = 20,
		.tENV_min = 20, .tSR = 20, .tRFS = 60, .tRP = 100,
		.tACK = 20, .tSS = 50, .tDZFS = 47
	},
	[3] = {
		.t2CYC = 86, .tCYC = 39, .tDS = 7, .tDH = 5, .tDVS = 20,
		.tDVH = 6, .tCVS = 20, .tCVH = 6, .tFS_min = 0,
		.tLI_max = 100, .tMLI = 20, .tAZ = 10, .tZAH = 20,
		.tENV_min = 20, .tSR = 20, .tRFS = 60, .tRP = 100,
		.tACK = 20, .tSS = 50, .tDZFS = 35
	},
	[4] = {
		.t2CYC = 57, .tCYC = 25, .tDS = 5, .tDH = 5, .tDVS = 7,
		.tDVH = 6, .tCVS = 7, .tCVH = 6, .tFS_min = 0,
		.tLI_max = 100, .tMLI = 20, .tAZ = 10, .tZAH = 20,
		.tENV_min = 20, .tSR = 50, .tRFS = 60, .tRP = 100,
		.tACK = 20, .tSS = 50, .tDZFS = 25
	},
	[5] = {
		.t2CYC = 38, .tCYC = 17, .tDS = 4, .tDH = 5, .tDVS = 5,
		.tDVH = 6, .tCVS = 10, .tCVH = 10, .tFS_min = 0,
		.tLI_max = 75, .tMLI = 20, .tAZ = 10, .tZAH = 20,
		.tENV_min = 20, .tSR = 20, .tRFS = 50, .tRP = 85,
		.tACK = 20, .tSS = 50, .tDZFS = 40
	},
};

#define NR_UDMA_SPECS (sizeof udma_specs / sizeof udma_specs[0])

struct fsl_ata_time_regs {
	u8 time_off, time_on, time_1, time_2w;
	u8 time_2r, time_ax, time_pio_rdx, time_4;
	u8 time_9, time_m, time_jn, time_d;
	u8 time_k, time_ack, time_env, time_rpx;
	u8 time_zah, time_mlix, time_dvh, time_dzfs;
	u8 time_dvs, time_cvh, time_ss, time_cyc;
};

static void update_timing_config(struct fsl_ata_time_regs *tp, struct ata_host *host)
{
	u32 *lp = (u32 *)tp;
	struct pata_fsl_priv *priv = host->private_data;
	u32 *ctlp = (u32 *)priv->fsl_ata_regs;
	int i;

	for (i = 0; i < 5; i++) {
		__raw_writel(*lp, ctlp);
		lp++;
		ctlp++;
	}
	mb();
}

/*!
 * Calculate values for the ATA bus timing registers and store
 * them into the hardware.
 *
 * @param       xfer_mode   specifies XFER xfer_mode
 * @param       pdev        specifies platform_device
 *
 * @return      EINVAL      speed out of range, or illegal mode
 */
static int set_ata_bus_timing(u8 xfer_mode, struct platform_device *pdev)
{
	struct fsl_ata_platform_data *plat = (struct fsl_ata_platform_data *)
					     pdev->dev.platform_data;
	struct ata_host *host = dev_get_drvdata(&pdev->dev);

	/* get the bus clock cycle time, in ns */
	int T = 1 * 1000 * 1000 * 1000 / plat->get_clk_rate();
	struct fsl_ata_time_regs tr = {0};

	/*
	 * every mode gets the same t_off and t_on
	 */
	tr.time_off = 3;
	tr.time_on = 3;

	if (xfer_mode >= XFER_UDMA_0) {
		int speed = xfer_mode - XFER_UDMA_0;
		if (speed >= NR_UDMA_SPECS) {
			return -EINVAL;
		}
		tr.time_ack = (udma_specs[speed].tACK + T) / T;
		tr.time_env = (udma_specs[speed].tENV_min + T) / T;
		tr.time_rpx = (udma_specs[speed].tRP + T) / T + 2;

		tr.time_zah = (udma_specs[speed].tZAH + T) / T;
		tr.time_mlix = (udma_specs[speed].tMLI + T) / T;
		tr.time_dvh = (udma_specs[speed].tDVH + T) / T + 1;
		tr.time_dzfs = (udma_specs[speed].tDZFS + T) / T;

		tr.time_dvs = (udma_specs[speed].tDVS + T) / T;
		tr.time_cvh = (udma_specs[speed].tCVH + T) / T;
		tr.time_ss = (udma_specs[speed].tSS + T) / T;
		tr.time_cyc = (udma_specs[speed].tCYC + T) / T;
	} else if (xfer_mode >= XFER_MW_DMA_0) {
		int speed = xfer_mode - XFER_MW_DMA_0;
		if (speed >= NR_MDMA_SPECS) {
			return -EINVAL;
		}
		tr.time_m = (mdma_specs[speed].tM + T) / T;
		tr.time_jn = (mdma_specs[speed].tJNH + T) / T;
		tr.time_d = (mdma_specs[speed].tD + T) / T;

		tr.time_k = (mdma_specs[speed].tKW + T) / T;
	} else {
		int speed = xfer_mode - XFER_PIO_0;
		if (speed >= NR_PIO_SPECS) {
			return -EINVAL;
		}
		tr.time_1 = (pio_specs[speed].t1 + T) / T;
		tr.time_2w = (pio_specs[speed].t2_8 + T) / T;

		tr.time_2r = (pio_specs[speed].t2_8 + T) / T;
		tr.time_ax = (pio_specs[speed].tA + T) / T + 2;
		tr.time_pio_rdx = 1;
		tr.time_4 = (pio_specs[speed].t4 + T) / T;

		tr.time_9 = (pio_specs[speed].t9 + T) / T;
	}

	update_timing_config(&tr, host);

	return 0;
}

static void pata_fsl_set_piomode(struct ata_port *ap, struct ata_device *adev)
{
	set_ata_bus_timing(adev->pio_mode, to_platform_device(ap->dev));
}

static void pata_fsl_set_dmamode(struct ata_port *ap, struct ata_device *adev)
{
	struct pata_fsl_priv *priv = ap->host->private_data;

	priv->ultra = adev->dma_mode >= XFER_UDMA_0;

	set_ata_bus_timing(adev->dma_mode, to_platform_device(ap->dev));
}

static int pata_fsl_port_start(struct ata_port *ap)
{
	return 0;
}

static void dma_callback(void *arg, int error_status, unsigned int count)
{
	struct ata_port *ap = arg;
	struct pata_fsl_priv *priv = ap->host->private_data;
	u8 *ata_regs = priv->fsl_ata_regs;

	priv->dma_done = 1;
	/* 
	 * DMA is finished, so unmask INTRQ from the drive to allow the
	 * normal ISR to fire.
	 */
	__raw_writel(FSL_ATA_INTR_ATA_INTRQ2, ata_regs + FSL_ATA_INT_EN);
	mb();
}

static void pata_fsl_bmdma_setup(struct ata_queued_cmd *qc)
{
	int nr_sg = 0;
	int chan;
	int dma_mode = 0, dma_ultra;
	u8 ata_control;
	struct ata_port *ap = qc->ap;
	struct pata_fsl_priv *priv = ap->host->private_data;
	u8 *ata_regs = priv->fsl_ata_regs;
	struct fsl_ata_platform_data *plat = ap->dev->platform_data;
	struct scatterlist tmp[plat->max_sg], *tsg, *sg;
	int err;

	priv->dma_dir = qc->dma_dir;

	/*
	 * Configure the on-chip ATA interface hardware.
	 */
	dma_ultra = priv->ultra ?
		FSL_ATA_CTRL_DMA_ULTRA : 0;

	ata_control = FSL_ATA_CTRL_FIFO_RST_B |
		      FSL_ATA_CTRL_ATA_RST_B |
		      FSL_ATA_CTRL_DMA_PENDING |
		      dma_ultra;

	if (qc->dma_dir == DMA_TO_DEVICE) {
		chan = priv->dma_wchan;
		ata_control |= FSL_ATA_CTRL_FIFO_TX_EN |
			      FSL_ATA_CTRL_DMA_WRITE;
		dma_mode = DMA_MODE_WRITE;
	} else {
		chan = priv->dma_rchan;
		ata_control |= FSL_ATA_CTRL_FIFO_RCV_EN;
		dma_mode = DMA_MODE_READ;
	}

	__raw_writel(ata_control, ata_regs + FSL_ATA_CONTROL);
	__raw_writel(plat->fifo_alarm, ata_regs + FSL_ATA_FIFO_ALARM);
	__raw_writel(FSL_ATA_INTR_ATA_INTRQ1, ata_regs + FSL_ATA_INT_EN);
	mb();

	/*
	 * Set up the DMA completion callback.
	 */
	mxc_dma_callback_set(chan, dma_callback, (void *)ap);

	/*
	 * Copy the sg list to an array.
	 */
	tsg = tmp;
	ata_for_each_sg(sg, qc) {
		memcpy(tsg, sg, sizeof *sg);
		tsg++;
		nr_sg++;
	}
	
	err = mxc_dma_sg_config(chan, tmp, nr_sg, 0, dma_mode);
	if (err) {
		printk(KERN_ERR "pata_fsl_bmdma_setup: error %d\n", err);
	}
}

static void pata_fsl_bmdma_start(struct ata_queued_cmd *qc)
{
	struct ata_port *ap = qc->ap;
	struct pata_fsl_priv *priv = ap->host->private_data;
	int chan;
	int err;

	/*
	 * Start the channel.
	 */
	chan = qc->dma_dir == DMA_TO_DEVICE ? priv->dma_wchan : priv->dma_rchan;

	priv->dma_done = 0;

	err = mxc_dma_enable(chan);
	if (err) {
		printk(KERN_ERR "%s: : error %d\n", __func__, err);
	}

	ap->ops->exec_command(ap, &qc->tf);
}

static void pata_fsl_bmdma_stop(struct ata_queued_cmd *qc)
{
	struct ata_port *ap = qc->ap;

	/* do a dummy read as in ata_bmdma_stop */
	ata_altstatus(ap);
}

static u8 pata_fsl_bmdma_status(struct ata_port *ap)
{
	struct pata_fsl_priv *priv = ap->host->private_data;

	return priv->dma_done ? ATA_DMA_INTR : 0;
}

static void pata_fsl_dma_init(struct ata_port *ap)
{
	struct pata_fsl_priv *priv = ap->host->private_data;

	priv->dma_rchan = -1;
	priv->dma_wchan = -1;

	priv->dma_rchan = mxc_dma_request(MXC_DMA_ATA_RX, "MXC ATA RX");
	if (priv->dma_rchan < 0) {
		dev_printk(KERN_ERR, ap->dev, "couldn't get RX DMA channel\n");
		goto err_out;
	}

	priv->dma_wchan = mxc_dma_request(MXC_DMA_ATA_TX, "MXC ATA TX");
	if (priv->dma_wchan < 0) {
		dev_printk(KERN_ERR, ap->dev, "couldn't get TX DMA channel\n");
		goto err_out;
	}       

	dev_printk(KERN_ERR, ap->dev, "rchan=%d wchan=%d\n", priv->dma_rchan,
		   priv->dma_wchan);
	return;

err_out:
	ap->mwdma_mask = 0;
	ap->udma_mask = 0;
	mxc_dma_free(priv->dma_rchan);
	mxc_dma_free(priv->dma_wchan);
	kfree(priv);
}

static u8 pata_fsl_irq_ack(struct ata_port *ap, unsigned int chk_drq)
{
	unsigned int bits = chk_drq ? ATA_BUSY | ATA_DRQ : ATA_BUSY;
	u8 status;

	status = ata_busy_wait(ap, bits, 1000);
	if (status & bits)
		if (ata_msg_err(ap))
			printk(KERN_ERR "abnormal status 0x%X\n", status);

	return status;
}

static void ata_dummy_noret(struct ata_port *ap) { return; }

static struct scsi_host_template pata_fsl_sht = {
	.module			= THIS_MODULE,
	.name			= DRV_NAME,
	.ioctl			= ata_scsi_ioctl,
	.queuecommand		= ata_scsi_queuecmd,
	.can_queue		= ATA_DEF_QUEUE,
	.this_id		= ATA_SHT_THIS_ID,
	.sg_tablesize		= LIBATA_MAX_PRD,
	.cmd_per_lun		= ATA_SHT_CMD_PER_LUN,
	.emulated		= ATA_SHT_EMULATED,
	.use_clustering		= ATA_SHT_USE_CLUSTERING,
	.proc_name		= DRV_NAME,
	.dma_boundary		= FSL_ATA_MAX_SG_LEN,
	.slave_configure	= ata_scsi_slave_config,
	.slave_destroy		= ata_scsi_slave_destroy,
	.bios_param		= ata_std_bios_param,
};

static struct ata_port_operations pata_fsl_port_ops = {
	.set_piomode		= pata_fsl_set_piomode,
	.set_dmamode		= pata_fsl_set_dmamode,

	.port_disable		= ata_port_disable,
	.tf_load		= ata_tf_load,
	.tf_read		= ata_tf_read,
	.check_status		= ata_check_status,
	.exec_command		= ata_exec_command,
	.dev_select		= ata_std_dev_select,

	.freeze			= ata_bmdma_freeze,
	.thaw			= ata_bmdma_thaw,
	.error_handler		= ata_bmdma_error_handler,
	.post_internal_cmd	= ata_bmdma_post_internal_cmd,
	.cable_detect		= ata_cable_unknown,

	.bmdma_setup		= pata_fsl_bmdma_setup,
	.bmdma_start		= pata_fsl_bmdma_start,

	.qc_prep		= ata_noop_qc_prep,
	.qc_issue		= ata_qc_issue_prot,

	.data_xfer		= ata_data_xfer_noirq,

	.irq_clear		= ata_dummy_noret,
	.irq_on			= ata_irq_on,
	.irq_ack		= pata_fsl_irq_ack,

	.port_start		= pata_fsl_port_start,

	.bmdma_stop		= pata_fsl_bmdma_stop,
	.bmdma_status		= pata_fsl_bmdma_status,
};

static void fsl_setup_port(struct ata_ioports *ioaddr)
{
	unsigned int shift = 2;

	ioaddr->data_addr	= ioaddr->cmd_addr + (ATA_REG_DATA    << shift);
	ioaddr->error_addr	= ioaddr->cmd_addr + (ATA_REG_ERR     << shift);
	ioaddr->feature_addr	= ioaddr->cmd_addr + (ATA_REG_FEATURE << shift);
	ioaddr->nsect_addr	= ioaddr->cmd_addr + (ATA_REG_NSECT   << shift);
	ioaddr->lbal_addr	= ioaddr->cmd_addr + (ATA_REG_LBAL    << shift);
	ioaddr->lbam_addr	= ioaddr->cmd_addr + (ATA_REG_LBAM    << shift);
	ioaddr->lbah_addr	= ioaddr->cmd_addr + (ATA_REG_LBAH    << shift);
	ioaddr->device_addr	= ioaddr->cmd_addr + (ATA_REG_DEVICE  << shift);
	ioaddr->status_addr	= ioaddr->cmd_addr + (ATA_REG_STATUS  << shift);
	ioaddr->command_addr	= ioaddr->cmd_addr + (ATA_REG_CMD     << shift);
}

/**
 *	pata_fsl_probe		-	attach a platform interface
 *	@pdev: platform device
 *
 *	Register a platform bus integrated ATA host controller
 *
 *	The 3 platform device resources are used as follows:
 *
 *		- I/O Base (IORESOURCE_MEM) virt. addr. of ATA controller regs
 *		- CTL Base (IORESOURCE_MEM) unused
 *		- IRQ	   (IORESOURCE_IRQ) platform IRQ assigned to ATA
 *
 */
static int __devinit pata_fsl_probe(struct platform_device *pdev)
{
	struct resource *io_res;
	struct ata_host *host;
	struct ata_port *ap;
	struct fsl_ata_platform_data *plat = (struct fsl_ata_platform_data *)
					     pdev->dev.platform_data;
	struct pata_fsl_priv *priv;
	u8 *ata_regs;

	/* 
	 * Get an ata_host structure for this device
	 */
	host = ata_host_alloc(&pdev->dev, 1);
	if (!host)
		return -ENOMEM;
	ap = host->ports[0];

	/* 
	 * Allocate private data
	 */
	priv = kzalloc(sizeof (struct pata_fsl_priv), GFP_KERNEL);
	if(priv == NULL) {
		/* free(host); */
		return -ENOMEM;
	}
	host->private_data = priv;

	/*
	 * Set up resources
	 */
	if (unlikely(pdev->num_resources != 3)) {
		dev_err(&pdev->dev, "invalid number of resources\n");
		return -EINVAL;
	}

	io_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	ata_regs = (u8 *)io_res->start;
	priv->fsl_ata_regs = ata_regs;
	ap->ioaddr.cmd_addr = (void *)(ata_regs + FSL_ATA_DRIVE_DATA);
	ap->ioaddr.ctl_addr = (void *)(ata_regs + FSL_ATA_DRIVE_CONTROL);
	ap->ioaddr.altstatus_addr = ap->ioaddr.ctl_addr;
	ap->ops = &pata_fsl_port_ops;
	ap->pio_mask = 0x7F;
	ap->mwdma_mask = 0x7F;
	ap->udma_mask = plat->udma_mask;
	pata_fsl_sht.sg_tablesize = plat->max_sg;

	fsl_setup_port(&ap->ioaddr);

	/*
	 * Do platform-specific initialization (e.g. allocate pins,
	 * turn on clock).  After this call it is assumed that
	 * plat->get_clk_rate() can be called to calculate
	 * timing.
	 */
	if (plat->init && plat->init(pdev)) {
		/* REVISIT: don't leak what ata_host_alloc() allocated */
		return -ENODEV;
	}

	/* Deassert the reset bit to enable the interface */
	__raw_writel(FSL_ATA_CTRL_ATA_RST_B, ata_regs + FSL_ATA_CONTROL);
	mb();

	/* Set initial timing and mode */
	set_ata_bus_timing(XFER_PIO_4, pdev);

	/* get DMA ready */
	pata_fsl_dma_init(ap);

	/*
	 * Enable the ATA INTRQ interrupt from the bus, but
	 * only allow the CPU to see it (INTRQ2) at this point.
	 * INTRQ1, which goes to the DMA, will be enabled later.
	 */
	__raw_writel(FSL_ATA_INTR_ATA_INTRQ2, ata_regs + FSL_ATA_INT_EN);
	mb();

	/* activate */
	return ata_host_activate(host, platform_get_irq(pdev, 0), ata_interrupt,
				 0, &pata_fsl_sht);
}

/**
 *	pata_fsl_remove	-	unplug a platform interface
 *	@pdev: platform device
 *
 *	A platform bus ATA device has been unplugged. Perform the needed
 *	cleanup. Also called on module unload for any active devices.
 */
static int __devexit pata_fsl_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct ata_host *host = dev_get_drvdata(dev);
	struct pata_fsl_priv *priv = host->private_data;
	struct fsl_ata_platform_data *plat = (struct fsl_ata_platform_data *)
					     pdev->dev.platform_data;
	u8 *ata_regs = priv->fsl_ata_regs;

	__raw_writel(0, ata_regs + FSL_ATA_INT_EN); /* Disable interrupts */
	mb();

	ata_host_detach(host);

	if (plat->exit)
		plat->exit();

	kfree(priv);

	return 0;
}

#ifdef CONFIG_PM
static int pata_fsl_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct ata_host *host = dev_get_drvdata(&pdev->dev);
	struct pata_fsl_priv *priv = host->private_data;
	struct fsl_ata_platform_data *plat = (struct fsl_ata_platform_data *)
					     pdev->dev.platform_data;
	u8 *ata_regs = priv->fsl_ata_regs;

	/* Disable interrupts. */
	__raw_writel(0, ata_regs + FSL_ATA_INT_EN);
	mb();

	if (plat->exit)
		plat->exit();

	return 0;
}

static int pata_fsl_resume(struct platform_device *pdev)
{
	struct ata_host *host = dev_get_drvdata(&pdev->dev);
	struct pata_fsl_priv *priv = host->private_data;
	struct fsl_ata_platform_data *plat = (struct fsl_ata_platform_data *)
					     pdev->dev.platform_data;
	u8 *ata_regs = priv->fsl_ata_regs;

	if (plat->init && plat->init(pdev)) {
		return -ENODEV;
	}

	/* Deassert the reset bit to enable the interface */
	__raw_writel(FSL_ATA_CTRL_ATA_RST_B, ata_regs + FSL_ATA_CONTROL);
	mb();

	/* Set initial timing and mode */
	set_ata_bus_timing(XFER_PIO_4, pdev);

	/*
	 * Enable hardware interrupts.
	 */
	__raw_writel(FSL_ATA_INTR_ATA_INTRQ2, ata_regs + FSL_ATA_INT_EN);
	mb();

	return 0;
}
#endif

static struct platform_driver pata_fsl_driver = {
	.probe		= pata_fsl_probe,
	.remove		= __devexit_p(pata_fsl_remove),
#ifdef CONFIG_PM
	.suspend	= pata_fsl_suspend,
	.resume		= pata_fsl_resume,
#endif
	.driver = {
		.name		= DRV_NAME,
		.owner		= THIS_MODULE,
	},
};

static int __init pata_fsl_init(void)
{
	return platform_driver_register(&pata_fsl_driver);

	return 0;
}

static void __exit pata_fsl_exit(void)
{
	platform_driver_unregister(&pata_fsl_driver);
}
module_init(pata_fsl_init);
module_exit(pata_fsl_exit);

MODULE_AUTHOR("Freescale Semiconductor, Inc.");
MODULE_DESCRIPTION("low-level driver for Freescale ATA");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);

/*
 * Navis
 * Author : Igor S.K.
 *
 * Derived from OMAP3530 EVM code by
 *	Manikandan Pillai <mani.pillai@ti.com>
 * Derived from Beagle Board and 3430 SDP code by
 *	Richard Woodruff <r-woodruff2@ti.com>
 *	Syed Mohammed Khasim <khasim@ti.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <twl4030.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/arch/mem.h>
/*#include <asm/arch/mux.h>*/
#include <asm/arch/sys_proto.h>
#include <i2c.h>
#include <asm/mach-types.h>
#include "orion.h"

static u8 omap3_evm_version;

u8 get_omap3_evm_rev(void)
{
	return omap3_evm_version;
}

static void omap3_evm_get_revision(void)
{
	unsigned int smsc_id;

	/* Ethernet PHY ID is stored at ID_REV register */
	smsc_id = readl(CONFIG_SMC911X_BASE + 0x50) & 0xFFFF0000;
	printf("Read back SMSC id 0x%x\n", smsc_id);

	switch (smsc_id) {
	/* SMSC9115 chipset */
	case 0x01150000:
		omap3_evm_version = OMAP3EVM_BOARD_GEN_1;
		break;
	/* SMSC 9220 chipset */
	case 0x92200000:
	default:
		omap3_evm_version = OMAP3EVM_BOARD_GEN_2;
       }
}

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */
	/* board id for Linux */
	gd->bd->bi_arch_number = MACH_TYPE_OMAP3ORION;
	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

	return 0;
}


static void vaux4_on()
{

    unsigned char byte;

    void * pcio1 = (void *)0x48002448;

    printf("VAUX4 Init ...");
    /* set VAUX4 to 2.5V */
    byte = TWL4030_PM_RECEIVER_DEV_GRP_P1;
    twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, byte,
            TWL4030_PM_RECEIVER_VAUX4_DEV_GRP);

    byte = TWL4030_PM_RECEIVER_VAUX4_VSEL_25;
    twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, byte,
            TWL4030_PM_RECEIVER_VAUX4_DEDICATED);

    sr32(pcio1, 27, 1, 0);
    sr32(pcio1, 26, 1, 1);
    sr32(pcio1, 25, 1, 1);
    sr32(pcio1, 24, 1, 0);


    printf(" done.\n");

}

extern int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
static void dsi_tmp_reset_fix()
{
    /* Check if we have VAUX3 pwr running */

    u8 byte, val1, val2;
    int reset_flag = 0;

    twl4030_i2c_read_u8(TWL4030_CHIP_PM_RECEIVER, &val1,
            TWL4030_PM_RECEIVER_VAUX3_DEV_GRP);
    twl4030_i2c_read_u8(TWL4030_CHIP_PM_RECEIVER, &val2,
            TWL4030_PM_RECEIVER_VAUX3_DEDICATED);

    byte = TWL4030_PM_RECEIVER_DEV_GRP_P1;
    twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, byte,
            TWL4030_PM_RECEIVER_VAUX3_DEV_GRP);

    byte = TWL4030_PM_RECEIVER_VAUX3_VSEL_28;
    twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, byte,
            TWL4030_PM_RECEIVER_VAUX3_DEDICATED);

    if (!((val1 & TWL4030_PM_RECEIVER_DEV_GRP_P1) &&
            val2 == TWL4030_PM_RECEIVER_VAUX3_VSEL_28))
    {

        printf("No VAUX3 active. Switch ON and reset.\n");
        do_reset(NULL, 0, 0, NULL);

        printf("Reset error???\n");
    }
    else
        printf("Active VAUX3 found. Go ahead.\n");
}

//#define I2C_ACCUM_DEBUG

#define SHUTDOWN twl4030_i2c_write_u8(TWL4030_CHIP_PM_MASTER, 0x01, TWL4030_PM_MASTER_P2_SW_EVENTS); \
		 twl4030_i2c_write_u8(TWL4030_CHIP_PM_MASTER, 0x01, TWL4030_PM_MASTER_P3_SW_EVENTS); \
		 twl4030_i2c_write_u8(TWL4030_CHIP_PM_MASTER, 0x01, TWL4030_PM_MASTER_P1_SW_EVENTS);



static void check_accum()
{
#ifdef I2C_ACCUM_DEBUG
	printf("\n   =========================================================================================	\n\n");
#endif

	if(i2c_get_bus_num() != 2 && i2c_set_bus_num(2) < 0)
	{
	    udelay (1000);
	    
	    if(i2c_set_bus_num(2) < 0)
		    printf("\nAccess to I2C num 2 failed!\n");
	}

	if(i2c_get_bus_num() == 2)
	{
#ifdef I2C_ACCUM_DEBUG
	    printf("Current I2C bus num is 2\n");
#endif
	    int error;

	    if(!i2c_probe (0x55))
	    {
		uchar volt1, volt2;
		uint  voltage = 0;
		int i;

		for(i = 0; i < 3; i++)
		{
		    if(!(error = smb_read(0x55, 0x09, &volt1)))
		    {
			voltage += (uint)volt1;
			voltage <<= 8;
			break;
		    }
		    else udelay (1000 + i * 10000);
		}

		for(i = 0; i < 3; i++)
		{
		    if(!(error = smb_read(0x55, 0x08, &volt2)))
		    {
			voltage |= (uint)volt2;
			break;
		    }
		    else udelay (1000 + i * 10000);
		}
#ifdef I2C_ACCUM_DEBUG
		printf("VALUE OF VOLTAGE ACCUM IS %d.%d V or 0x%02x\n", voltage/1000, voltage%1000, voltage);
#else
		printf("VALUE OF VOLTAGE ACCUM IS %d.%d V\n", voltage/1000, voltage%1000);
#endif

///udelay(50000);
//musb_platform_init();

		if(voltage <= 3200)
		{
		    printf("\tRequires charging...\n", voltage);
		
		    if(!i2c_probe (0x09))
		    {
			uchar value;
			
			for(i = 0; i < 3; i++)
			{
			    if(!(error = smb_read(0x09, 0x04, &value)))
								    break;
			    else udelay (1000 + i * 10000);
			}
#ifdef I2C_ACCUM_DEBUG
			printf("Value of register4 is = 0x%02x =\n", value);
#endif
			
			if(!error)
			{
//			    if((value & (1 << 7)) && (value & (1 << 6)))
			    if(value & 0xC0)
			    {
				error = smb_write (0x09, 0x02, 0xFF);
#ifdef I2C_ACCUM_DEBUG
				if(!error)
				     printf("REG2 was writed\n");
				else printf("REG2 don't writed... ERROR = %d\n", error);

				if(!(error = smb_read(0x09, 0x02, &value)))
				     printf("Value of register2 is = 0x%02x =\n", value);
				else printf("register2 test of CHGR FAILED... ERROR = %d\n", error);
#endif
				error = smb_write (0x09, 0x01, 0xAF);
#ifdef I2C_ACCUM_DEBUG
				if(!error)
				     printf("REG1 was writed\n");
				else printf("REG1 don't writed... ERROR = %d\n", error);

				if(!(error = smb_read(0x09, 0x01, &value)))
				     printf("Value of register1 is = 0x%02x =\n", value);
				else printf("register1 test of CHGR FAILED... ERROR = %d\n", error);
#endif
				error = smb_write (0x09, 0x00, 0x61);
#ifdef I2C_ACCUM_DEBUG
				if(!error)
				     printf("REG0 was writed\n");
				else printf("REG0 don't writed... ERROR = %d\n", error);

				if(!(error = smb_read(0x09, 0x00, &value)))
				     printf("Value of register0 is = 0x%02x =\n", value);
				else printf("register0 test of CHGR FAILED... ERROR = %d\n", error);
#endif
			    }
			    else
			    {
				printf("\nSHUTDOWN!\n");
				i2c_set_bus_num(0);
				SHUTDOWN
			    }
			}
			else printf("REG4 test of CHGR FAILED... ERROR = %d\n", error);
		    }
		    else printf("I2C TEST CHRG-CONTROLLER FAILED...\n");
		}
	    }
	    else printf("I2C TEST FG FAILED...\n");
	}
	
	if(i2c_get_bus_num()) i2c_set_bus_num(0);
#ifdef I2C_ACCUM_DEBUG
	printf("\n   =========================================================================================\n\n");
#endif
}

/*
 * Routine: misc_init_r
 * Description: Init ethernet (done here so udelay works)
 */
int misc_init_r(void)
{
#ifdef CONFIG_DRIVER_OMAP34XX_I2C
    dsi_tmp_reset_fix();
    /*
     * We have to enable this VAUX4 LDO since there's a buggy chip
     * in i2c-2 on this board that needs this power.
     * Otherwise it occupies entire i2c-2 bus with some
     * garbage. The culprit is TBFound.
     */
    vaux4_on();
    i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
#endif

    check_accum();

#if defined(CONFIG_CMD_NET)
	setup_net_chip();
#endif

	dieid_num_r();

	return 0;
}

/*
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers specific to the
 *		hardware. Many pins need to be moved from protect to primary
 *		mode.
 */
void set_muxconf_regs(void)
{
	/*MUX_ORION();*/
}

/*
 * Routine: setup_net_chip
 * Description: Setting up the configuration GPMC registers specific to the
 *		Ethernet hardware.
 */
static void setup_net_chip(void)
{
	struct gpio *gpio3_base = (struct gpio *)OMAP34XX_GPIO3_BASE;
	struct gpio *gpio1_base = (struct gpio *)OMAP34XX_GPIO1_BASE;
	struct ctrl *ctrl_base = (struct ctrl *)OMAP34XX_CTRL_BASE;

	/* Configure GPMC registers */
	writel(NET_GPMC_CONFIG1, &gpmc_cfg->cs[5].config1);
	writel(NET_GPMC_CONFIG2, &gpmc_cfg->cs[5].config2);
	writel(NET_GPMC_CONFIG3, &gpmc_cfg->cs[5].config3);
	writel(NET_GPMC_CONFIG4, &gpmc_cfg->cs[5].config4);
	writel(NET_GPMC_CONFIG5, &gpmc_cfg->cs[5].config5);
	writel(NET_GPMC_CONFIG6, &gpmc_cfg->cs[5].config6);
	writel(NET_GPMC_CONFIG7, &gpmc_cfg->cs[5].config7);

	/* Enable off mode for NWE in PADCONF_GPMC_NWE register */
	writew(readw(&ctrl_base ->gpmc_nwe) | 0x0E00, &ctrl_base->gpmc_nwe);
	/* Enable off mode for NOE in PADCONF_GPMC_NADV_ALE register */
	writew(readw(&ctrl_base->gpmc_noe) | 0x0E00, &ctrl_base->gpmc_noe);
	/* Enable off mode for ALE in PADCONF_GPMC_NADV_ALE register */
	writew(readw(&ctrl_base->gpmc_nadv_ale) | 0x0E00,
		&ctrl_base->gpmc_nadv_ale);

	/* determine omap3evm revision */
	omap3_evm_get_revision();

	if ( get_omap3_evm_rev() == OMAP3EVM_BOARD_GEN_1 ){
		/* Make GPIO 64 as output pin */
		writel(readl(&gpio3_base->oe) & ~(GPIO0), &gpio3_base->oe);

		/* Now send a pulse on the GPIO pin */
		writel(GPIO0, &gpio3_base->setdataout);
		udelay(1);
		writel(GPIO0, &gpio3_base->cleardataout);
		udelay(1);
		writel(GPIO0, &gpio3_base->setdataout);
	}else{
		/* Make GPIO 07 as output pin */
		writel(readl(&gpio1_base->oe) & ~(GPIO7), &gpio1_base->oe);

		/* Now send a pulse on the GPIO pin */
		writel(GPIO7, &gpio1_base->setdataout);
		udelay(1);
		writel(GPIO7, &gpio1_base->cleardataout);
		udelay(1);
		writel(GPIO7, &gpio1_base->setdataout);
	}

}

int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_SMC911X
	rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);
#endif
	return rc;
}

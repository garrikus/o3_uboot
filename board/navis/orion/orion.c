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

#include "font.h"

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


static void vaux4_on(void)
{
	unsigned char byte;

	void *pcio1 = (void *)0x48002448;

	printf("VAUX4 Init   ...");
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

/*
#define gpio180_bit			20
#define GPIO_OE_bank6			0x49058034
#define GPIO_DATAOUT_bank6		0x4905803C
*/
inline void reset_for_dsi(void)
{
	if (getenv("board")) {
		do_reset(NULL, 0, 0, NULL);
		printf("Reset error???\n");
	} else {

		/*
		    r32setv(GPIO_OE_bank6,      gpio180_bit, 1, 0);		//GPIO6 GPIO_OE - to out
		    r32setv(GPIO_DATAOUT_bank6, gpio180_bit, 1, 0);		//OFF
		    udelay(1000);
		    r32setv(GPIO_DATAOUT_bank6, gpio180_bit, 1, 1);		//ON
		*/
		struct gpio *gpio6_base = (struct gpio *)OMAP34XX_GPIO6_BASE;

		/* Make GPIO 180 as output pin */
		writel(readl(&gpio6_base->oe) & ~(GPIO20), &gpio6_base->oe);

		/*
		 *  Now send a pulse on the GPIO pin
		 *
		 *  For hardware reset timings see
		 *  HX8369-A-DS datasheet. It suggests
		 *  t_resw = 10usec (min)
		 *  t_rest = 120msec (max)
		 *  We give a bit more.
		 */

		writel(GPIO20, &gpio6_base->setdataout);
		udelay(20);
		writel(GPIO20, &gpio6_base->cleardataout);
		udelay(20);
		writel(GPIO20, &gpio6_base->setdataout);
		udelay(130000);
	}
}

extern int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
static void vaux3_on(void)
{
	if (getenv("board")) {
		/* Check if we have VAUX3 pwr running */

		u8 byte, val1, val2;

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
		      val2 == TWL4030_PM_RECEIVER_VAUX3_VSEL_28)) {
			u32 *addr = (u32 *)(0x4020eff0);
			int i;

			for (i = 0; i < 4; i++, addr++) {
				if (*addr != 0xafafafaf) {
					printf("No VAUX3 active. Switch ON and reset.\n");
					reset_for_dsi();
				}
			}

			printf("Active VAUX3 found. Go ahead.\n");
		} else
			printf("Active VAUX3 found. Go ahead.\n");
	} else {
		u8 byte;

		printf("VAUX3 Init   ...");

		byte = TWL4030_PM_RECEIVER_DEV_GRP_P1;
		twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, byte,
		                     TWL4030_PM_RECEIVER_VAUX3_DEV_GRP);
		byte = TWL4030_PM_RECEIVER_VAUX3_VSEL_28;
		twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, byte,
		                     TWL4030_PM_RECEIVER_VAUX3_DEDICATED);

		reset_for_dsi();
		printf(" done.\n");
	}
}

void frame_reset(void);
void sym_to_frame(int, int, symbol *);
void string_to_frame(int, int, symbol *, int);
int panel_init(void);
int panel_update(void);

static void set_string_to_display(void)
{
	frame_reset();

	unsigned char a[27][3] = {
		{0},
		{0},
		{0},
		{0},
		{0},
		{_______X, XXXXX___, ________},
		{_____XXX, XXXXXXX_, ________},
		{____XXX_, ____XXXX, ________},
		{___XXX__, _____XXX, X_______},
		{___XXX__, ______XX, X_______},
		{________, ______XX, X_______},
		{________, ____XXXX, X_______},
		{________, XXXXXXXX, X_______},
		{______XX, XX____XX, X_______},
		{___XXXX_, ______XX, X_______},
		{__XXX___, ______XX, X_______},
		{_XXX____, ______XX, X_______},
		{_XXX____, ______XX, X_______},
		{_XXX____, ______XX, X_______},
		{_XXX____, ______XX, X_______},
		{_XXX____, ______XX, XX______},
		{__XXXX__, ____XXXX, XXX_____},
		{____XXXX, XXXXXX__, _XXXXX__},
		{______XX, XXXX____, ___XXX__},
		{0},
		{0},
		{0}
	};

	unsigned char z[27][3] = {
		{0},
		{0},
		{0},
		{0},
		{0},
		{________, XXXXXX__, ________},
		{______XX, XXXXXXX_, ________},
		{_____XXX, _____XXX, ________},
		{____XXX_, ______XX, X_______},
		{____XXX_, ______XX, X_______},
		{________, ______XX, X_______},
		{________, ____XXXX, ________},
		{_______X, XXXXXXX_, ________},
		{_______X, XXXXXX__, ________},
		{________, ____XXXX, ________},
		{________, ______XX, X_______},
		{________, ______XX, XX______},
		{________, ______XX, XX______},
		{________, ______XX, XX______},
		{__XXX___, ______XX, XX______},
		{__XXX___, ______XX, X_______},
		{__XXX___, _____XXX, ________},
		{____XXX_, ____XXX_, ________},
		{______XX, XXXXX___, ________},
		{0},
		{0},
		{0}
	};

	unsigned char g[27][3] = {
		{0},
		{0},
		{0},
		{0},
		{0},
		{____XXXX, XXXXXXXX, XX______},
		{____XXXX, XXXXXXXX, XX______},
		{____XXX_, ________, ________},
		{____XXX_, ________, ________},
		{____XXX_, ________, ________},
		{____XXX_, ________, ________},
		{____XXX_, ________, ________},
		{____XXX_, ________, ________},
		{____XXX_, ________, ________},
		{____XXX_, ________, ________},
		{____XXX_, ________, ________},
		{____XXX_, ________, ________},
		{____XXX_, ________, ________},
		{____XXX_, ________, ________},
		{____XXX_, ________, ________},
		{____XXX_, ________, ________},
		{____XXX_, ________, ________},
		{____XXX_, ________, ________},
		{____XXX_, ________, ________},
		{0},
		{0},
		{0}
	};

	unsigned char r[27][3] = {
		{0},
		{0},
		{0},
		{0},
		{0},
		{____XXX_, ___XXX__, ________},
		{____XXX_, _XXXXXXX, ________},
		{____XXXX, XX____XX, X_______},
		{____XXXX, _______X, XX______},
		{____XXX_, ________, XXX_____},
		{____XXX_, ________, XXX_____},
		{____XXX_, ________, XXX_____},
		{____XXX_, ________, XXX_____},
		{____XXX_, ________, XXX_____},
		{____XXX_, ________, XXX_____},
		{____XXX_, ________, XXX_____},
		{____XXX_, ________, XXX_____},
		{____XXXX, _______X, XX______},
		{____XXXX, X_____XX, X_______},
		{____XXXX, XXXXXXX_, ________},
		{____XXX_, __XXX___, ________},
		{____XXX_, ________, ________},
		{____XXX_, ________, ________},
		{____XXX_, ________, ________},
		{____XXX_, ________, ________},
		{____XXX_, ________, ________},
		{____XXX_, ________, ________},
	};

	unsigned char u[27][3] = {
		{0},
		{0},
		{0},
		{0},
		{0},
		{____XXX_, ________, __XXX___},
		{____XXX_, ________, __XXX___},
		{_____XXX, ________, _XXX____},
		{_____XXX, ________, _XXX____},
		{______XX, X_______, XXX_____},
		{______XX, X_______, XXX_____},
		{_______X, XX_____X, XX______},
		{_______X, XX_____X, XX______},
		{________, XXX___XX, X_______},
		{________, XXX___XX, X_______},
		{________, _XXX_XXX, ________},
		{________, __XXXXXX, ________},
		{________, ___XXXX_, ________},
		{________, ____XXX_, ________},
		{________, ___XXX__, ________},
		{________, ___XXX__, ________},
		{________, __XXX___, ________},
		{________, __XXX___, ________},
		{________, _XXX____, ________},
		{________, _XXX____, ________},
		{____XXX_, XXX_____, ________},
		{_____XXX, X_______, ________}
	};

	unsigned char k[27][3] = {
		{0},
		{0},
		{0},
		{0},
		{0},
		{____XXX_, ________, XXX_____},
		{____XXX_, _______X, XX______},
		{____XXX_, ______XX, X_______},
		{____XXX_, _____XXX, ________},
		{____XXX_, ____XXX_, ________},
		{____XXX_, ___XXX__, ________},
		{____XXX_, __XXX___, ________},
		{____XXX_, _XXX____, ________},
		{____XXXX, XXX_____, ________},
		{____XXXX, XXXX____, ________},
		{____XXXX, X_XXX___, ________},
		{____XXXX, ___XXX__, ________},
		{____XXX_, ___XXX__, ________},
		{____XXX_, ____XXX_, ________},
		{____XXX_, _____XXX, ________},
		{____XXX_, _____XXX, ________},
		{____XXX_, ______XX, X_______},
		{____XXX_, _______X, XX______},
		{____XXX_, ________, XXX_____},
		{0},
		{0},
		{0}
	};

	unsigned char s[27][3] = {
		{0},
		{0},
		{0},
		{0},
		{0},
		{________, _XXXXXXX, X_______},
		{_______X, XXXXXXXX, XXX_____},
		{______XX, X_______, _XXX____},
		{_____XXX, ________, __XXX___},
		{____XXX_, ________, __XXX___},
		{____XXX_, ________, __XXX___},
		{____XXX_, ________, __XXX___},
		{____XXX_, ________, ________},
		{____XXX_, ________, ________},
		{____XXX_, ________, ________},
		{____XXX_, ________, ________},
		{____XXX_, ________, ________},
		{____XXX_, ________, ________},
		{____XXX_, ________, ________},
		{____XXX_, ________, __XXX___},
		{_____XXX, ________, __XXX___},
		{______XX, X_______, _XXX____},
		{_______X, XXXXXXXX, XXX_____},
		{________, _XXXXXXX, X_______},
		{0},
		{0},
		{0}
	};

	unsigned char i[27][3] = {
		{0},
		{0},
		{0},
		{0},
		{0},
		{____XXX_, ________, ___XXX__},
		{____XXX_, ________, ___XXX__},
		{____XXX_, ________, ___XXX__},
		{____XXX_, ________, __XXXX__},
		{____XXX_, ________, _XXXXX__},
		{____XXX_, ________, XX_XXX__},
		{____XXX_, _______X, X__XXX__},
		{____XXX_, ______XX, ___XXX__},
		{____XXX_, _____XX_, ___XXX__},
		{____XXX_, ____XX__, ___XXX__},
		{____XXX_, ___XX___, ___XXX__},
		{____XXX_, __XX____, ___XXX__},
		{____XXX_, _XX_____, ___XXX__},
		{____XXX_, XX______, ___XXX__},
		{____XXXX, X_______, ___XXX__},
		{____XXXX, ________, ___XXX__},
		{____XXX_, ________, ___XXX__},
		{____XXX_, ________, ___XXX__},
		{____XXX_, ________, ___XXX__},
		{0},
		{0},
		{0}
	};

	unsigned char t[27][3] = {
		{0},
		{0},
		{0},
		{0},
		{0},
		{____XXXX, XXXXXXXX, XXXXX___},
		{____XXXX, XXXXXXXX, XXXXX___},
		{________, ___XXX__, ________},
		{________, ___XXX__, ________},
		{________, ___XXX__, ________},
		{________, ___XXX__, ________},
		{________, ___XXX__, ________},
		{________, ___XXX__, ________},
		{________, ___XXX__, ________},
		{________, ___XXX__, ________},
		{________, ___XXX__, ________},
		{________, ___XXX__, ________},
		{________, ___XXX__, ________},
		{________, ___XXX__, ________},
		{________, ___XXX__, ________},
		{________, ___XXX__, ________},
		{________, ___XXX__, ________},
		{________, ___XXX__, ________},
		{________, ___XXX__, ________},
		{0},
		{0},
		{0}
	};

	unsigned char m[27][3] = {
		{0},
		{0},
		{0},
		{0},
		{0},
		{____XXX_, ________, ____XXX_},
		{____XXX_, ________, ____XXX_},
		{____XXX_, ________, ____XXX_},
		{____XXXX, ________, ___XXXX_},
		{____XXXX, X_______, __XXXXX_},
		{____XXX_, XX______, _XX_XXX_},
		{____XXX_, _XX_____, XX__XXX_},
		{____XXX_, __XX___X, X___XXX_},
		{____XXX_, ___XX_XX, ____XXX_},
		{____XXX_, ____XXX_, ____XXX_},
		{____XXX_, _____X__, ____XXX_},
		{____XXX_, ________, ____XXX_},
		{____XXX_, ________, ____XXX_},
		{____XXX_, ________, ____XXX_},
		{____XXX_, ________, ____XXX_},
		{____XXX_, ________, ____XXX_},
		{____XXX_, ________, ____XXX_},
		{____XXX_, ________, ____XXX_},
		{____XXX_, ________, ____XXX_},
		{0},
		{0},
		{0}
	};

	unsigned char y[27][3] = {
		{0},
		{0},
		{0},
		{0},
		{0},
		{____XXX_, ________, ____XXX_},
		{____XXX_, ________, ____XXX_},
		{____XXX_, ________, ____XXX_},
		{____XXX_, ________, ____XXX_},
		{____XXX_, ________, ____XXX_},
		{____XXX_, ________, ____XXX_},
		{____XXX_, ________, ____XXX_},
		{____XXX_, ________, ____XXX_},
		{____XXXX, XXXXX___, ____XXX_},
		{____XXXX, XXXXXXX_, ____XXX_},
		{____XXX_, _____XXX, ____XXX_},
		{____XXX_, _______X, X___XXX_},
		{____XXX_, _______X, X___XXX_},
		{____XXX_, _______X, X___XXX_},
		{____XXX_, _______X, X___XXX_},
		{____XXX_, _______X, X___XXX_},
		{____XXX_, _____XXX, ____XXX_},
		{____XXXX, XXXXXXX_, ____XXX_},
		{____XXXX, XXXX____, ____XXX_},
		{0},
		{0},
		{0}
	};

	unsigned char e[27][3] = {
		{0},
		{0},
		{0},
		{0},
		{0},
		{________, _XXXXXXX, X_______},
		{_______X, XXXXXXXX, XXX_____},
		{______XX, X_______, _XXX____},
		{_____XXX, ________, __XXX___},
		{____XXX_, ________, __XXX___},
		{____XXX_, ________, __XXX___},
		{____XXX_, ________, __XXX___},
		{____XXXX, XXXXXXXX, XXXXX___},
		{____XXXX, XXXXXXXX, XXXXX___},
		{____XXX_, ________, ________},
		{____XXX_, ________, ________},
		{____XXX_, ________, ________},
		{____XXX_, ________, ________},
		{____XXX_, ________, ________},
		{____XXX_, ________, __XXX___},
		{_____XXX, ________, __XXX___},
		{______XX, X_______, _XXX____},
		{_______X, XXXXXXXX, XXX_____},
		{________, _XXXXXXX, X_______},
		{0},
		{0},
		{0}
	};

	unsigned char dot[24][1] = {
		{0}, {0}, {0}, {0}, {0},
		{0}, {0}, {0}, {0}, {0},
		{0}, {0}, {0}, {0}, {0},
		{0}, {0}, {0}, {0}, {0},
		{_XXXX___},
		{XXXXXX__},
		{XXXXXX__},
		{_XXXX___},
	};

	symbol str[15] = {
		{&z[0][0], 24, 27, 3},
		{&a[0][0], 24, 27, 3},
		{&g[0][0], 24, 27, 3},
		{&r[0][0], 20, 27, 3},
		{&u[0][0], 20, 27, 3},
		{&z[0][0], 22, 27, 3},
		{&k[0][0], 20, 27, 3},
		{&a[0][0], 22, 27, 3},

		{&s[0][0], 20, 27, 3},
		{&i[0][0], 24, 27, 3},
		{&s[0][0], 24, 27, 3},
		{&t[0][0], 20, 27, 3},
		{&e[0][0], 20, 27, 3},
		{&m[0][0], 24, 27, 3},
		{&y[0][0], 28, 27, 3}
	};

	string_to_frame(150, 100, str, 8);
	string_to_frame(140, 170, str + 8, 7);

	symbol tt = {&dot[0][0], 14, 24, 1};

	sym_to_frame(313, 170, &tt);     //32
	sym_to_frame(327, 170, &tt);     //14
	sym_to_frame(341, 170, &tt);     //14
}

#define BUFF_ADDR		"0x80000000"

unsigned get_addr(unsigned);

static void set_picture_to_display(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	char maddr[10], msize[10], *img_size = "0x00177000";

	enum {
	        magic = 4,
	        img1,
	        img2,
	        img3,
	        img4
	} blocks;

	sprintf(maddr, "%x", get_addr((unsigned int)magic));
	sprintf(msize, "%x", (get_addr((unsigned int)img1) - get_addr((unsigned int)magic)));

	unsigned magnum;
	char *s[] = {"do_nand", "read", BUFF_ADDR, maddr, msize};

	do_nand(NULL, 0, 5, s);
	magnum = readl(simple_strtoul(BUFF_ADDR, NULL, 16));

	if (magnum == gd->mnumber) {
		sprintf(maddr, "%x", (unsigned int)gd->fb_base);
		sprintf(msize, "%x", get_addr((unsigned int)img1));
		s[2] = maddr;
		s[3] = msize;
		s[4] = img_size;
		/* load_pic_to_frame */
		do_nand(NULL, 0, 5, s);
	} else
		set_string_to_display();

	udelay(5000);               // by 1000 it works but has low bright

	if (panel_init()) {
		puts("DSS ERROR:  panel don't init!\n");
		return;
	}

	puts("Panel Init   ... done.\n");

	if (panel_update()) {
		puts("DSS ERROR:  panel don't update!\n");
		return;
	}

	puts("Panel Update ... done.\n");

	if (magnum == gd->mnumber) {
		/* load_pic_to_frame */
		sprintf(msize, "%x", get_addr((unsigned int)img2));
		s[3] = msize;
		do_nand(NULL, 0, 5, s);
	}
}

#define U_BOOT_VERSION_ADDR		0x4020f040

inline void save_version(void)
{
	extern const char version_string[];
	char *addr = (char *)(U_BOOT_VERSION_ADDR);
	int i, len = strlen(version_string) > 60 ? 60 : strlen(version_string);

	for (i = 0; i < len; i++)
		addr[i] = version_string[i];

	addr[i] = '\0';
}

/*
 * Routine: misc_init_r
 * Description: Init ethernet (done here so udelay works)
 */
int misc_init_r(void)
{
#ifdef CONFIG_DRIVER_OMAP34XX_I2C
	vaux3_on();
	set_picture_to_display();
	/*
	 * We have to enable this VAUX4 LDO since there's a buggy chip
	 * in i2c-2 on this board that needs this power.
	 * Otherwise it occupies entire i2c-2 bus with some
	 * garbage. The culprit is TBFound.
	 */
	vaux4_on();
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
	save_version();
#endif

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

	if ( get_omap3_evm_rev() == OMAP3EVM_BOARD_GEN_1 ) {
		/* Make GPIO 64 as output pin */
		writel(readl(&gpio3_base->oe) & ~(GPIO0), &gpio3_base->oe);

		/* Now send a pulse on the GPIO pin */
		writel(GPIO0, &gpio3_base->setdataout);
		udelay(1);
		writel(GPIO0, &gpio3_base->cleardataout);
		udelay(1);
		writel(GPIO0, &gpio3_base->setdataout);
	} else {
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

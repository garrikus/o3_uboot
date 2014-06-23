#include <twl4030.h>
#include <asm/io.h>
#include <asm/arch-omap3/sys_proto.h>

#ifdef CONFIG_SYS_HUSH_PARSER
#include <hush.h>
#endif


#define START_ADDR_RAM		0x80000000
#define END_ADDR_RAM		0x8FE3FC4F

typedef volatile unsigned char rbyte;


//The DDR test from address 0x80000000 to 0x8fe3fc4f   -------------------------------------------

int do_test_ram(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	rbyte *base_addr = (rbyte *)START_ADDR_RAM;
	rbyte *end_addr  = (rbyte *)END_ADDR_RAM;
	u32 nBytes = ((u32)end_addr - (u32)base_addr);
	extern signed long test_sdram(rbyte *, u32);

	printf("\nSDRAM Start address 0x%x, End address 0x%x\n\n",
	       (unsigned int)base_addr, (unsigned int)end_addr);

	if (test_sdram((rbyte *)base_addr, nBytes))
		printf(">TEST OK\r\n\n");
	else
		printf(">TEST ERROR\r\n\n");

	return 0;
}

U_BOOT_CMD(
        test_ram,	1,		1,	do_test_ram,
        "print result for SDRAM test",
        ""
);


//#define DEBUG_CHECK_PWR
//All power sources test   -----------------------------------------------------------------------

#define TWL4030_PM_RECEIVER_VAUX1_VSEL_30			0x04

int do_mod_power(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (argc < 2)
		goto usage;
	else if (argc < 3) {
		if (strncmp(argv[1], "off", 3) == 0) {
			int i;
			u8 value = 0x01;

			puts("The shutting of ORION is down!\n");
			i2c_set_bus_num(0);

			for (i = 0; i < 3; i++)
				i2c_write(TWL4030_CHIP_PM_MASTER, TWL4030_PM_MASTER_P1_SW_EVENTS + i, 1, &value, 1);
		} else
			goto usage;
	}

	if (i2c_get_bus_num()) i2c_set_bus_num(0);

	char *module = argv[1];
	unsigned char vsel, dev_grp, dedicated, remap, grp_p = TWL4030_PM_RECEIVER_DEV_GRP_P1;

	if (strncmp(module, "vaux1", 5) == 0) {
		vsel      = TWL4030_PM_RECEIVER_VAUX1_VSEL_30;
		dev_grp   = TWL4030_PM_RECEIVER_VAUX1_DEV_GRP;
		dedicated = TWL4030_PM_RECEIVER_VAUX1_DEDICATED;
		remap     = TWL4030_PM_RECEIVER_VAUX1_REMAP;
	} else if (strncmp(module, "vaux3", 5) == 0) {
		vsel      = TWL4030_PM_RECEIVER_VAUX3_VSEL_28;
		dev_grp   = TWL4030_PM_RECEIVER_VAUX3_DEV_GRP;
		dedicated = TWL4030_PM_RECEIVER_VAUX3_DEDICATED;
		remap     = TWL4030_PM_RECEIVER_VAUX3_REMAP;
	} else if (strncmp(module, "vaux4", 5) == 0) {
		vsel      = TWL4030_PM_RECEIVER_VAUX4_VSEL_25;
		dev_grp   = TWL4030_PM_RECEIVER_VAUX4_DEV_GRP;
		dedicated = TWL4030_PM_RECEIVER_VAUX4_DEDICATED;
		remap     = TWL4030_PM_RECEIVER_VAUX4_REMAP;
	} else if (strncmp(module, "mmc", 3) == 0) {
		vsel      = TWL4030_PM_RECEIVER_VMMC1_VSEL_30;
		dev_grp   = TWL4030_PM_RECEIVER_VMMC1_DEV_GRP;
		dedicated = TWL4030_PM_RECEIVER_VMMC1_DEDICATED;
		remap     = TWL4030_PM_RECEIVER_VMMC1_REMAP;
	} else if (strncmp(module, "vdac", 4) == 0) {
		vsel      = TWL4030_PM_RECEIVER_VDAC_VSEL_18;
		dev_grp   = TWL4030_PM_RECEIVER_VDAC_DEV_GRP;
		dedicated = TWL4030_PM_RECEIVER_VDAC_DEDICATED;
		remap     = TWL4030_PM_RECEIVER_VDAC_REMAP;
	} else if (strncmp(module, "vpll2", 5) == 0) {
		vsel      = TWL4030_PM_RECEIVER_VPLL2_VSEL_18;
		dev_grp   = TWL4030_PM_RECEIVER_VPLL2_DEV_GRP;
		dedicated = TWL4030_PM_RECEIVER_VPLL2_DEDICATED;
		remap     = TWL4030_PM_RECEIVER_VPLL2_REMAP;
		grp_p     = TWL4030_PM_RECEIVER_DEV_GRP_ALL;
	} else
		goto usage;

	if (strncmp(argv[2], "up", 2) == 0)
		twl4030_pmrecv_vsel_cfg(dedicated, vsel, dev_grp, grp_p);
	else if (strncmp(argv[2], "down", 4) == 0)
		twl4030_pmrecv_vsel_cfg(remap, 0, dev_grp, 0);
	else if (strncmp(argv[2], "check", 5) == 0) {
		int i;
		unsigned char val1, val2;

		for (i = 0; i < 3; i++) {
			if (!(twl4030_i2c_read_u8(TWL4030_CHIP_PM_RECEIVER, &val1, dev_grp)))
				break;
			else
				udelay(1000 + i * 1000);
		}

		if (i == 3) {
			printf("Check failed! I2C error... Try again!\n ");
			return -1;
		}

		for (i = 0; i < 3; i++) {
			if (!(twl4030_i2c_read_u8(TWL4030_CHIP_PM_RECEIVER, &val2, dedicated)))
				break;
			else
				udelay(1000 + i * 1000);
		}

		if (i == 3) {
			printf("Check failed! I2C error... Try again!\n ");
			return -1;
		}

		if (!((val1 & TWL4030_PM_RECEIVER_DEV_GRP_P1) && val2 == vsel)) {
			if (argc < 4) printf("No %s active.\n", module);
			return 0;
		} else {
			if (argc < 4) printf("Active %s found.\n", module);
			return 1;
		}
	} else
		goto usage;

	return 0;

usage:
	cmd_usage(cmdtp);
	return 1;
}

U_BOOT_CMD(
        power,  4,              1,      do_mod_power,
        "this command provides control for modules power",
        "<module> up    - turn on the power supply of the module\n"
        "power <module> down  - turn off the power supply of the module\n"
        "power <module> check - check the power supply of the module\n"
        "the list of modules: vaux1, vaux3, vaux4, mmc, vdac, vpll2.\n"
        "power off            - this command is shutting the device down\n"
);


int do_test_pwr(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char *s[]      = {"do_mod_power", "vaux1", "up"};
	char *module[] = {"vaux1", "vdac", "vpll2", "mmc", "vaux3", "vaux4"};
	char *cmd[]    = {"up", "check", "down"};

	int i, j;

	for (j = 0; j < 2; j++) {
		s[2] = cmd[j];

		for (i = 0; i < 6; i++) {
			s[1] = module[i];
			do_mod_power(NULL, 0, 3, s);
		}
	}

	printf("\nTesting...\n");

	for (i = 0; i < 40000; i++) udelay(1000);

	printf("\n>TEST DONE\r\n\n");

	s[1] = "vaux1";
	s[2] = "down";
	do_mod_power(NULL, 0, 3, s);
	s[1] = "vaux3";
	do_mod_power(NULL, 0, 3, s);

	return 0;
}

U_BOOT_CMD(
        test_pwr_level,	1,		1,	do_test_pwr,
        "run POWER test",
        ""
);


//The test of the clock function   ---------------------------------------------------------------

static u32 test_clk()
{
	u32 val, count = 0;
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;
	struct prm *prm_base = (struct prm *)PRM_BASE;
	struct gptimer *gpt4_base = (struct gptimer *)OMAP34XX_GPT4;
	struct gptimer *gpt3_base = (struct gptimer *)OMAP34XX_GPT3;

	val = readl(&prm_base->clksrc_ctrl);

	if (!(val & SYSCLKDIV_2))
		writel((val & (~(3 << 6))) | (1 << 6), &prm_base->clksrc_ctrl);

	val = readl(&prcm_base->fclken_per) | (1 << 5) | (1 << 4);	//FCLK GPtimer4 & GPtimer3 is enabled
	writel(val, &prcm_base->fclken_per);

	sr32(&prm_base->clksel, 1, 2, 0x3);				// sys clk = 26 MHz

	sr32(&prcm_base->clksel_per, 2, 1, 0x1);			// GPT4 = sys clk /
	sr32(&prcm_base->clksel_per, 1, 1, 0x0);			// GPT3 = sys 32k /

	writel(0, &gpt4_base->tldr);					// start counting at 0
	writel(0, &gpt4_base->tcrr);					// set GPT4 to 0
	writel(0, &gpt3_base->tldr);					// start counting at 0
	writel(0, &gpt3_base->tcrr);					// set GPT3 to 0

	writel(0x3, &gpt3_base->tclr);					// enable clock - START GPT3
	writel(0x3, &gpt4_base->tclr);					// enable clock - START GPT4

	while (readl(&gpt3_base->tcrr) < 327680);

	count = readl(&gpt4_base->tcrr);

	writel(0, &gpt4_base->tclr);					// STOP GPT4
	writel(0, &gpt3_base->tclr);					// STOP GPT3
	writel(0, &gpt4_base->tcrr);					// set GPT4 to 0
	writel(0, &gpt3_base->tcrr);					// set GPT3 to 0

	return count;
}

int do_test_freq(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	puts("\nWait for test a few second ...\n");
	u32 count = test_clk();

	if (count > 259948000 && count < 260052000) 	printf("\n>TEST OK\r\n");
	else						printf("\n>TEST ERROR\r\n");

	return 0;
}

U_BOOT_CMD(
        test_freq,	1,		1,	do_test_freq,
        "run CLOCK test and print result",
        ""
);


//The command for check the Linux kernel from NAND memory   --------------------------------------

int check_nand_kernel(void)
{
	int argc = 5;
	char *arg[5] = {"nand", "read", "0x82000000", "280000", "400000"};

	if (!do_nand(NULL, 0, argc, arg))
		if (check_kernel_img())
			return 1;

	return 0;
}

int do_test_nand(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (check_nand_kernel())
		puts(">TEST OK\r\n");
	else
		puts(">TEST ERROR\r\n");

	return 0;
}

U_BOOT_CMD(
        test_nand, 1,	1,	do_test_nand,
        "run NAND test and print result",
        ""
);


//The command for boot Linux in test mode   ------------------------------------------------------

int do_test_runlin(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char *s = getenv("nandargs");

	strcat(s, " o2testmode\0");

	const char str[strlen(s)];	//without this string 's' is incorrect in 'setenv()'!

	strcpy((char *)str, (const char *)s);
	setenv("nandargs", (char *)str);
	printf("\n>TEST DONE\n");

	parse_string_outer(getenv("bootcmd"), FLAG_PARSE_SEMICOLON | FLAG_EXIT_FROM_LOOP);

	return 0;
}

U_BOOT_CMD(
        test_runlin,	1,		1,	do_test_runlin,
        "boot Linux in test mode",
        ""
);

int do_img(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (argc < 2)
		goto usage;

	DECLARE_GLOBAL_DATA_PTR;

	char *cmd = argv[1];
	enum {
		magic = 4,
		img1,
		img2,
		img3,
		img4,
	};

	if (strncmp(cmd, "magic", 5) == 0) {
		if (argc < 3)
			goto usage;

		char *buff_addr = "0x80000000";
		char maddr[10], msize[10];

		sprintf(maddr, "%x", get_addr((unsigned int)magic));
		sprintf(msize, "%x", (get_addr((unsigned int)img1) - get_addr((unsigned int)magic)));

		if (strncmp(argv[2], "check", 5) == 0) {
			unsigned bytes;
			char *s[] = {"do_nand", "read", buff_addr, maddr, msize};

			do_nand(NULL, 0, 5, s);
			bytes = readl(simple_strtoul(s[2], NULL, 16));

			printf("%s: 0x%x\n", bytes == gd->mnumber ? \
			       "The magic is the same" : "The magic does not match", bytes);
		} else if (strncmp(argv[2], "write", 5) == 0) {
			char tmpdata[10];
			sprintf(tmpdata, "%x", gd->mnumber);
			char *s[] = {"do_mem_mw", buff_addr, tmpdata, "", ""}; 		//magic = 0xaf7254db

			if (argc > 3)
				s[2] = argv[3];

			do_mem_mw(NULL, 0, 3, s);
			s[0] = "do_nand";
			s[1] = "erase";
			s[2] = maddr;
			s[3] = msize;
			do_nand(NULL, 0, 4, s);
			s[1] = "write";
			s[2] = buff_addr;
			s[3] = maddr;
			s[4] = msize;
			do_nand(NULL, 0, 5, s);
		} else goto usage;
	} else if (strncmp(cmd, "show", 4) == 0) {
		if (argc < 3)
			goto usage;
		char framebuff[10], img_addr[10], *img_size = "0x180000";
		sprintf(framebuff, "%x", gd->fb_base);
		char *s[] = {"do_nand", "read", framebuff, "", img_size};

		if (strncmp(argv[2], "1", 1) == 0) sprintf(img_addr, "%x", get_addr((unsigned)img1));
		else if (strncmp(argv[2], "2", 1) == 0) sprintf(img_addr, "%x", get_addr((unsigned)img2));
		else if (strncmp(argv[2], "3", 1) == 0) sprintf(img_addr, "%x", get_addr((unsigned)img3));
		else if (strncmp(argv[2], "4", 1) == 0) sprintf(img_addr, "%x", get_addr((unsigned)img4));
		else goto usage;

		s[3] = img_addr;

		do_nand(NULL, 0, 5, s);
		panel_update();
	} else goto usage;

	return 0;

usage:
	cmd_usage(cmdtp);
	return 1;
}

U_BOOT_CMD(
        img,    4,              1,      do_img,
        "an operation with Image",
        "magic check - reads from NAND image magic number and checks him\n"
        "img magic write [magicnumber] - writes image magic number to NAND\n"
        "img show <img number/address> - reads image from NAND and output to LCD\n"
        "\t\tnumber = 1, 2, 3 or 4; address must be prefixed with '0x'\n"
);

int do_panel(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (argc < 2)
		goto usage;

	if (strncmp(argv[1], "init", 4) == 0) {
		if (panel_init()) {
			puts("DSS ERROR:  panel don't init!\n");
			return 1;
		} else
			puts("Panel Init ... done.\n");
	} else if (strncmp(argv[1], "update", 6) == 0) {
		if (panel_update()) {
			puts("DSS ERROR:  panel don't update!\n");
			return 1;
		} else
			puts("Panel Update ... done.\n");
	} else if (strncmp(argv[1], "on", 2) == 0) {
		panel_backlight(1);
		puts("the command the backlight ON was sent...\n");
	} else if (strncmp(argv[1], "off", 3) == 0) {
		panel_backlight(0);
		puts("the command the backlight OFF was sent...\n");
	} else if (strncmp(argv[1], "uninit", 6) == 0)
		panel_uninit();
	else if (strncmp(argv[1], "reset", 5) == 0) {
		extern void reset_for_dsi(void);
		reset_for_dsi();
		puts("the command the reset DSI was sent...\n");
	} else
		goto usage;

	return 0;

usage:
	cmd_usage(cmdtp);
	return 1;
}

U_BOOT_CMD(
        panel,  2,              1,      do_panel,
        "the panel control",
        "init   - initialize Display SubSystem\n"
        "panel update - output the image to screen from the framebuffer (0x8fc00000)\n"
        "panel on     - on the backlight\n"
        "panel off    - off the backlight\n"
        "panel reset  - hard reset DSI\n"
);

int do_setmode(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	DECLARE_GLOBAL_DATA_PTR;

	if (argc < 2) {
		if (gd->flags & GD_FLG_TEST_MODE)
			goto usage;
		else {
			gd->flags |= GD_FLG_TEST_MODE;
			puts("test mode is set.\n");
			return 0;
		}
	}

	if (!strncmp(argv[1], "dinamit", sizeof(argv[1]))) {
		gd->flags &= ~GD_FLG_TEST_MODE;
		puts("Welcome!!!\n");
	} else
		goto usage;

	return 0;

usage:
	puts("command is incorrect!\n");
	return 1;
}

U_BOOT_CMD(
        setmode,  2,              1,      do_setmode,
        "",
        "\n"
        "\n"
);


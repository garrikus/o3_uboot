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
//	sdram_test();		//function in EVM3530_ITBOK/diagnostics/src/dg_ram.c - nothing to return
	rbyte* base_addr = (rbyte*)START_ADDR_RAM;
	rbyte* end_addr  = (rbyte*)END_ADDR_RAM;
	u32 nBytes = ((u32)end_addr - (u32)base_addr);
	extern signed long test_sdram(rbyte*, u32);

	printf("\nSDRAM Start address 0x%x, End address 0x%x\n\n",
		(unsigned int)base_addr, (unsigned int)end_addr);

	if(test_sdram((rbyte*)base_addr, nBytes))// == SUCCESS)
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

int do_test_pwr(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    unsigned int i = 0;
    unsigned char val1, val2;

    if(i2c_get_bus_num()) i2c_set_bus_num(0);

    //	VMPU	1.2 V
    
    //	VCORE	1.2 V
/*    
    //	VIO	1.8 V
	twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VDAC_DEDICATED,	//	0X99
				TWL4030_PM_RECEIVER_VIO_VSEL,		//	0XAF
				TWL4030_PM_RECEIVER_VIO_DEV_GRP,	//	0XA6
				TWL4030_PM_RECEIVER_DEV_GRP_P1);	//	0X20
*/

    //	VAUX1	3.0 V
	twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VAUX1_DEDICATED,	//	0X75
				0x04,//TWL4030_PM_RECEIVER_VAUX1_VSEL_30,
				TWL4030_PM_RECEIVER_VAUX1_DEV_GRP,	//	0X72
				TWL4030_PM_RECEIVER_DEV_GRP_P1);	//	0X20

	for(i = 0; i < 3; i++)
	{
	    if(!(twl4030_i2c_read_u8(TWL4030_CHIP_PM_RECEIVER,
				&val1,
				TWL4030_PM_RECEIVER_VAUX1_DEV_GRP)))
									break;
	    else udelay (1000 + i * 10000);
	}

	for(i = 0; i < 3; i++)
	{
	    if(!(twl4030_i2c_read_u8(TWL4030_CHIP_PM_RECEIVER,
				&val2,
				TWL4030_PM_RECEIVER_VAUX1_DEDICATED)))
									break;
	    else udelay (1000 + i * 10000);
	}

	if (!((val1 & TWL4030_PM_RECEIVER_DEV_GRP_P1) &&
		val2 == 0x04))
#ifndef DEBUG_CHECK_PWR
						    i = 1;
#else
						    printf("No VAUX1 active.\n");
	else
    						    printf("Active VAUX1 found.\n");
#endif

    //	VDAC	1.8 V
	twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VDAC_DEDICATED,	//	0X99
				TWL4030_PM_RECEIVER_VDAC_VSEL_18,	//	0X03
				TWL4030_PM_RECEIVER_VDAC_DEV_GRP,	//	0X96
				TWL4030_PM_RECEIVER_DEV_GRP_P1);	//	0X20

	for(i = 0; i < 3; i++)
	{
	    if(!(twl4030_i2c_read_u8(TWL4030_CHIP_PM_RECEIVER,
				&val1,
				TWL4030_PM_RECEIVER_VDAC_DEV_GRP)))
									break;
	    else udelay (1000 + i * 10000);
	}

	for(i = 0; i < 3; i++)
	{
	    if(!(twl4030_i2c_read_u8(TWL4030_CHIP_PM_RECEIVER,
				&val2,
				TWL4030_PM_RECEIVER_VDAC_DEDICATED)))
									break;
	    else udelay (1000 + i * 10000);
	}

	if (!((val1 & TWL4030_PM_RECEIVER_DEV_GRP_P1) &&
		val2 == TWL4030_PM_RECEIVER_VDAC_VSEL_18))
#ifndef DEBUG_CHECK_PWR
						    i = 1;
#else
						    printf("No VDAC active.\n");
	else
    						    printf("Active VDAC found.\n");
#endif

    //	VPLL2	1.8 V
	twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VPLL2_DEDICATED,	//	0X91
				TWL4030_PM_RECEIVER_VPLL2_VSEL_18,	//	0X05
				TWL4030_PM_RECEIVER_VPLL2_DEV_GRP,	//	0X8E
				TWL4030_PM_RECEIVER_DEV_GRP_ALL);	//	0XE0

	for(i = 0; i < 3; i++)
	{
	    if(!(twl4030_i2c_read_u8(TWL4030_CHIP_PM_RECEIVER,
				&val1,
				TWL4030_PM_RECEIVER_VPLL2_DEV_GRP)))
									break;
	    else udelay (1000 + i * 10000);
	}

	for(i = 0; i < 3; i++)
	{
	    if(!(twl4030_i2c_read_u8(TWL4030_CHIP_PM_RECEIVER,
				&val2,
				TWL4030_PM_RECEIVER_VPLL2_DEDICATED)))
									break;
	    else udelay (1000 + i * 10000);
	}

	if (!((val1 & TWL4030_PM_RECEIVER_DEV_GRP_P1) &&
		val2 == TWL4030_PM_RECEIVER_VPLL2_VSEL_18))
#ifndef DEBUG_CHECK_PWR
						    i = 1;
#else
						    printf("No VPLL2 active.\n");
	else
    						    printf("Active VPLL2 found.\n");
#endif

    //	VMMC	3.0 V
	twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VMMC1_DEDICATED,	//	0X85
				TWL4030_PM_RECEIVER_VMMC1_VSEL_30,	//	0X02
				TWL4030_PM_RECEIVER_VMMC1_DEV_GRP,	//	0X82
				TWL4030_PM_RECEIVER_DEV_GRP_P1);	//	0X20

	for(i = 0; i < 3; i++)
	{
	    if(!(twl4030_i2c_read_u8(TWL4030_CHIP_PM_RECEIVER,
				&val1,
				TWL4030_PM_RECEIVER_VMMC1_DEV_GRP)))
									break;
	    else udelay (1000 + i * 10000);
	}

	for(i = 0; i < 3; i++)
	{
	    if(!(twl4030_i2c_read_u8(TWL4030_CHIP_PM_RECEIVER,
				&val2,
				TWL4030_PM_RECEIVER_VMMC1_DEDICATED)))
									break;
	    else udelay (1000 + i * 10000);
	}

	if (!((val1 & TWL4030_PM_RECEIVER_DEV_GRP_P1) &&
		val2 == TWL4030_PM_RECEIVER_VMMC1_VSEL_30))
#ifndef DEBUG_CHECK_PWR
						    i = 1;
#else
						    printf("No VMMC1 active.\n");
	else
    						    printf("Active VMMC1 found.\n");
#endif

    //	VAUX3	2.8 V
	twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VAUX3_DEDICATED,	//	0X7D
				TWL4030_PM_RECEIVER_VAUX3_VSEL_28,	//	0X03
				TWL4030_PM_RECEIVER_VAUX3_DEV_GRP,	//	0X7A
				TWL4030_PM_RECEIVER_DEV_GRP_P1);	//	0X20

	for(i = 0; i < 3; i++)
	{
	    if(!(twl4030_i2c_read_u8(TWL4030_CHIP_PM_RECEIVER,
				&val1,
				TWL4030_PM_RECEIVER_VAUX3_DEV_GRP)))
									break;
	    else udelay (1000 + i * 10000);
	}

	for(i = 0; i < 3; i++)
	{
	    if(!(twl4030_i2c_read_u8(TWL4030_CHIP_PM_RECEIVER,
				&val2,
				TWL4030_PM_RECEIVER_VAUX3_DEDICATED)))
									break;
	    else udelay (1000 + i * 10000);
	}

	if (!((val1 & TWL4030_PM_RECEIVER_DEV_GRP_P1) &&
		val2 == TWL4030_PM_RECEIVER_VAUX3_VSEL_28))
#ifndef DEBUG_CHECK_PWR
						    i = 1;
#else
						    printf("No VAUX3 active.\n");
	else
    						    printf("Active VAUX3 found.\n");
#endif

    //	VAUX4	2.5 V
	twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VAUX4_DEDICATED,	//	0X81
				TWL4030_PM_RECEIVER_VAUX4_VSEL_25,	//	0X07
				TWL4030_PM_RECEIVER_VAUX4_DEV_GRP,	//	0X7E
				TWL4030_PM_RECEIVER_DEV_GRP_P1);	//	0X20

	for(i = 0; i < 3; i++)
	{
	    if(!(twl4030_i2c_read_u8(TWL4030_CHIP_PM_RECEIVER,
				&val1,
				TWL4030_PM_RECEIVER_VAUX4_DEV_GRP)))
									break;
	    else udelay (1000 + i * 10000);
	}

	for(i = 0; i < 3; i++)
	{
	    if(!(twl4030_i2c_read_u8(TWL4030_CHIP_PM_RECEIVER,
				&val2,
				TWL4030_PM_RECEIVER_VAUX4_DEDICATED)))
									break;
	    else udelay (1000 + i * 10000);
	}

	if (!((val1 & TWL4030_PM_RECEIVER_DEV_GRP_P1) &&
		val2 == TWL4030_PM_RECEIVER_VAUX4_VSEL_25))
#ifndef DEBUG_CHECK_PWR
						    i = 1;
#else
						    printf("No VAUX4 active.\n");
	else
    						    printf("Active VAUX4 found.\n");
#endif
	if(!i)
	{
		printf("\n>TEST START!\n");

		for(; i < 40000; i++) udelay(1000);

		i = 0;
	}

	twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VAUX1_DEDICATED,	//	0X75
				0x04,//TWL4030_PM_RECEIVER_VAUX1_VSEL_30,
				TWL4030_PM_RECEIVER_VAUX1_DEV_GRP,	//	0X72
				0);	//	0X20
/*
	twl4030_i2c_read_u8(TWL4030_CHIP_PM_RECEIVER,
			    &val1,
			    TWL4030_PM_RECEIVER_VAUX1_DEV_GRP);
        twl4030_i2c_read_u8(TWL4030_CHIP_PM_RECEIVER,
    			    &val2,
			    TWL4030_PM_RECEIVER_VAUX1_DEDICATED);

	if (!((val1 & TWL4030_PM_RECEIVER_DEV_GRP_P1) &&
		val2 == 0x04))
						    printf("No VAUX1 active.\n");
	else
    						    printf("Active VAUX1 found.\n");
*/
	twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VAUX3_DEDICATED,
				TWL4030_PM_RECEIVER_VAUX3_VSEL_28,
				TWL4030_PM_RECEIVER_VAUX3_DEV_GRP,
				0);
/*
	twl4030_i2c_read_u8(TWL4030_CHIP_PM_RECEIVER,
			    &val1,
        		    TWL4030_PM_RECEIVER_VAUX3_DEV_GRP);

        twl4030_i2c_read_u8(TWL4030_CHIP_PM_RECEIVER,
    			     &val2,
        		    TWL4030_PM_RECEIVER_VAUX3_DEDICATED);

	if (!((val1 & TWL4030_PM_RECEIVER_DEV_GRP_P1) &&
		val2 == TWL4030_PM_RECEIVER_VAUX3_VSEL_28))
						    printf("No VAUX3 active.\n");
	else
    						    printf("Active VAUX3 found.\n");
*/
	if(!i) printf("\n>TEST DONE\r\n\n");
	else  printf("\nAny sources is not found... Try again!\r\n\n");

	return i;
}

U_BOOT_CMD(
	test_pwr_level,	1,		1,	do_test_pwr,
	"run POWER test and print result",
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

	sr32(&prm_base->clksel, 1, 2, 0x3);		// sys clk = 26 MHz

	sr32(&prcm_base->clksel_per, 2, 1, 0x1);	// GPT4 = sys clk /
	sr32(&prcm_base->clksel_per, 1, 1, 0x0);	// GPT3 = sys 32k /

	writel(0, &gpt4_base->tldr);			// start counting at 0
	writel(0, &gpt4_base->tcrr);			// set GPT4 to 0
	writel(0, &gpt3_base->tldr);			// start counting at 0
	writel(0, &gpt3_base->tcrr);			// set GPT3 to 0

	writel(0x3, &gpt3_base->tclr);			// enable clock - START GPT3
	writel(0x3, &gpt4_base->tclr);			// enable clock - START GPT4

	while(readl(&gpt3_base->tcrr) < 327680);

	count = readl(&gpt4_base->tcrr);

	writel(0, &gpt4_base->tclr);			// STOP GPT4
	writel(0, &gpt3_base->tclr);			// STOP GPT3
	writel(0, &gpt4_base->tcrr);			// set GPT4 to 0
	writel(0, &gpt3_base->tcrr);			// set GPT3 to 0

	return count;
}

int do_test_freq(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	puts("\nWait for test a few second ...\n");
	u32 count = test_clk();

	if(count > 259948000 && count < 260052000) 	printf("\n>TEST OK\r\n");
	else						printf("\n>TEST ERROR\r\n");
//printf("\n>count = %d\r\n", count);
	return 0;
}

U_BOOT_CMD(
	test_freq,	1,		1,	do_test_freq,
	"run CLOCK test and print result",
	""
);


//The command for check the Linux kernel from NAND memory   --------------------------------------

int do_test_nand(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	argc = 5;
	char* arg[5] = {"nand", "read", "0x82000000", "280000", "400000"};

	if(!do_nand(cmdtp, 0, argc, arg))
			if(check_kernel_img())
			{
				puts(">TEST OK\r\n");
				return 0;
			}

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
	char* s = getenv("nandargs");

	strcat(s, " o2testmode\0");

	const char str[strlen(s)];	//without this string 's' is incorrect in 'setenv()'!

	strcpy((char*)str, (const char*)s);
	setenv("nandargs", (char*)str);
	printf("\n>TEST DONE\n");

	parse_string_outer(getenv("bootcmd"), FLAG_PARSE_SEMICOLON | FLAG_EXIT_FROM_LOOP);

	return 0;
}

U_BOOT_CMD(
	test_runlin,	1,		1,	do_test_runlin,
	"boot Linux in test mode",
	""
);

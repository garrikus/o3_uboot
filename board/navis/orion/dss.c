#include <asm/io.h>
#include <asm/arch-omap3/sys_proto.h>
#include "display.h"

//#define DSS_DEBUG_OUTPUT
#define DSS_ERROR_OUTPUT


inline void r32setv(void*, u8, u8, u32);
inline void setv32(u32*, u8, u8, u32);

void putc(char);
void puts(const char*);
void udelay(unsigned long);
void printf(const char*, ...);

int wait_for_bit(volatile void* rg, u8 offset, u8 count, u32 data, u32 timeout)
{
    int t = 0;
    u32 value,
        mask = (1 << count);
    mask--;
    offset++;
    offset -= count;
    data <<= offset;
    mask <<= offset;

    while(1) {
        value = readl(rg);

        if((value & data) == data && ((value & mask) | data) == data)
                                                        return 0;

        if(++t > timeout)
                    return 1;

        udelay(1);
    }

    return 0;
}

void dssmsg(const char* s)
{
#ifdef DSS_DEBUG_OUTPUT
    if(s[0] != '<')
        puts("\tDSS DBG:  ");

    puts(s);
    putc('\n');
#endif
}

void dsserr(const char* s)
{
#ifdef DSS_ERROR_OUTPUT
    puts("DSS ERROR:  ");
    puts(s);
    putc('\n');
#endif
}


void dss_clocks(bool enable)
{
    struct dss_cm_registers* dcm_base  = (struct dss_cm_registers*)DSS_CM_BASE;

//    if(enable) {
///set DSS1_ALWON_FCLK
        writel(0x9, &dcm_base->clksel);                 //DPLL4 M4 divide factor for DSS1_ALWON_FCLK is 7
        r32setv(&dcm_base->fclken, EN_DSS1, 1, (u32)enable);//0x1);    //Enable the DSS1_ALWON_FCLK
///set DSS2_ALWON_FCLK
        r32setv(&dcm_base->fclken, EN_DSS2, 1, (u32)enable);//0x1);    //Enable the DSS2_ALWON_FCLK
///set DSS_L3_ICLK and DSS_L4_ICLK
        r32setv(&dcm_base->iclken, EN_DSS,  1, (u32)enable);//0x1);//Enable the DSS_L3_ICLK and 
									//DSS_L4_ICLK interface clocks
        r32setv(&dcm_base->autoidle, AUTO_DSS, 1, (u32)enable);//0x1);           //Ensable autoidle mode

        r32setv(&dcm_base->sleepdep, EN_IVA2, 3, 0x6);            //Domain sleep is disabled
        r32setv(&dcm_base->clkstctrl, CLKTRCTRL_DSS, 2, 0x3);     //Enable automatic transition
        r32setv(&dcm_base->fclken, EN_TV, 1, (u32)enable);//0x1);
//    } else {
//        r32setv(&dcm_base->fclken, EN_DSS1, 1, 0x0);
//        r32setv(&dcm_base->fclken, EN_DSS2, 1, 0x0);
//        r32setv(&dcm_base->fclken, EN_TV, 1, 0x0);
//        r32setv(&dcm_base->iclken, EN_DSS,  1, 0x0);
//    }
}

int dss_reset(void)
{
    struct display_subsystem_registers*  dss   = (struct display_subsystem_registers*)DISPLAY_SUBSYSTEM_BASE;
    struct display_controller_registers* dispc = (struct display_controller_registers*)DISPLAY_CONTROLLER_BASE;

    if(readl(&dispc->control) & 1) {
                r32setv(&dispc->control, 0, 1, 0);
                r32setv(&dispc->irqstatus, 0, 1, 1);

                if(wait_for_bit(&dispc->irqstatus, 0, 1, 1, 100000))
							dsserr("a frame is not done!");
//                else
//			dssmsg("a frame is done... ok");
    }

    dss_clocks(ENABLE);

    r32setv(&dss->sysconfig, SOFT_RESET, 1, 1);

    if(wait_for_bit(&dss->sysstatus, 0, 1, 1, 100000)) {
						dss_clocks(DISABLE);
						return 1;
    }

    dssmsg(" DSS reset is complete..................... ok");

    return 0;
}

static void set_lcd_display_type(struct orion_display* d)
{
//        enable_clocks(1);
    r32setv(&d->dispc->control, 3, 1, (u32)d->display_type);
//        enable_clocks(0);
}

static void set_parallel_interface_mode(struct orion_display* d)
{
    u32 value;
    int stallmode;
    int gpout0 = 1;
    int gpout1;

    switch(d->interface_mode) {
        case PARALLELMODE_BYPASS:
                                stallmode = 0;
                                gpout1    = 1;
                                break;

        case PARALLELMODE_RFBI:
                                stallmode = 1;
                                gpout1    = 0;
                                break;

        case PARALLELMODE_DSI:
                                stallmode = 1;
                                gpout1    = 1;
                                break;

        default:
                dsserr("wrong value of interface mode!");
                return;
    }

//    enable_clocks(1);

    value = readl(&d->dispc->control);

    setv32(&value, 11, 1, (u32)stallmode);
    setv32(&value, 15, 1, (u32)gpout0);
    setv32(&value, 16, 1, (u32)gpout1);

    writel(value, &d->dispc->control);
//    enable_clocks(0);
}

static void set_fifohandcheck(struct orion_display* d)
{
//    enable_clocks(1);
    r32setv(&d->dispc->control, 16, 1, (u32)d->fifohandcheck);
//    enable_clocks(0);
}

static void set_tft_data_lines(struct orion_display* d)
{
//    enable_clocks(1);
    r32setv(&d->dispc->control, 9, 2, (u32)d->color_depth);
//    enable_clocks(0);
}

#define cpu_is_omap24xx()               0


static int is_lcd_timings_error(int hsw, int hfp, int hbp,
            			int vsw, int vfp, int vbp)
{
    if (cpu_is_omap24xx()/* || omap_rev() < OMAP3430_REV_ES3_0*/) {
                if (hsw < 1 || hsw > 64  ||
                    hfp < 1 || hfp > 256 ||
                    hbp < 1 || hbp > 256 ||
                    vsw < 1 || vsw > 64  ||
                    vfp < 0 || vfp > 255 ||
                    vbp < 0 || vbp > 255)
            			            return 1;
    } else {
                if (hsw < 1 || hsw > 256  ||
                    hfp < 1 || hfp > 4096 ||
                    hbp < 1 || hbp > 4096 ||
                    vsw < 1 || vsw > 256  ||
                    vfp < 0 || vfp > 4095 ||
                    vbp < 0 || vbp > 4095)
		                            return 1;
    }

    return 0;
}

//setvalue(u32 source, u8 offset, u8 count, u32 data)

static void set_timings(struct orion_display* d)
{
    u32 timing_h, timing_v;

    if (cpu_is_omap24xx()/* || omap_rev() < OMAP3430_REV_ES3_0*/) {
                setv32(&timing_h, 5, 6,  d->timings.hsw-1);
		setv32(&timing_h, 15, 8, d->timings.hfp-1);
		setv32(&timing_h, 27, 8, d->timings.hbp-1);

		setv32(&timing_v, 5, 6,  d->timings.vsw-1);
		setv32(&timing_h, 15, 8, d->timings.vfp-1);
                setv32(&timing_h, 27, 8, d->timings.vbp-1);
    } else {
    		setv32(&timing_h, 7, 8,   d->timings.hsw-1);
		setv32(&timing_h, 19, 12, d->timings.hfp-1);
		setv32(&timing_h, 31, 12, d->timings.hbp-1);

		setv32(&timing_v, 7, 8,   d->timings.vsw-1);
		setv32(&timing_h, 19, 12, d->timings.vfp-1);
		setv32(&timing_h, 31, 12, d->timings.vbp-1);
    }

//    enable_clocks(1);
    writel(timing_h, &d->dispc->timing_h);
    writel(timing_v, &d->dispc->timing_v);
//    enable_clocks(0);
}

static void set_lcd_size(struct orion_display* d)
{
    if((d->timings.x_res > (1 << 11)) ||
       (d->timings.y_res > (1 << 11))) {
				dsserr("wrong lcd size!");
				return;
    }

    u32 val;

    setv32(&val, 26, 11, d->timings.y_res - 1);
    setv32(&val, 10, 11, d->timings.x_res  - 1);
//    enable_clocks(1);
    writel(val, &d->dispc->size_lcd);
//    enable_clocks(0);
}

static void set_lcd_timings(struct orion_display* d)
{
    if(is_lcd_timings_error(d->timings.hsw, d->timings.hfp,
                            d->timings.hbp, d->timings.vsw,
                            d->timings.vfp, d->timings.vbp)) {
						dsserr("wrong timings!");
						return;
    }

    set_timings(d);
    set_lcd_size(d);
/*
    unsigned xtot, ytot;
    unsigned long ht, vt;

    xtot = timings->x_res + timings->hfp + timings->hsw + timings->hbp;
    ytot = timings->y_res + timings->vfp + timings->vsw + timings->vbp;

    ht = (timings->pixel_clock * 1000) / xtot;
    vt = (timings->pixel_clock * 1000) / xtot / ytot;

        DSSDBG("xres %u yres %u\n", timings->x_res, timings->y_res);
        DSSDBG("pck %u\n", timings->pixel_clock);
        DSSDBG("hsw %d hfp %d hbp %d vsw %d vfp %d vbp %d\n",
                        timings->hsw, timings->hfp, timings->hbp,
                        timings->vsw, timings->vfp, timings->vbp);

        DSSDBG("hsync %luHz, vsync %luHz\n", ht, vt);
*/
}

int init_dispc(struct orion_display* d)
{
//    omap_dispc_register_isr(dsi_framedone_irq_callback, NULL,
//                            DISPC_IRQ_FRAMEDONE);

    set_lcd_display_type(d);				//LCD_DISPLAY_TFT);
    set_parallel_interface_mode(d);			//PARALLELMODE_DSI);
    set_fifohandcheck(d);
    set_tft_data_lines(d);				//24);
    set_lcd_timings(d);
    
    dssmsg(" DISPC init done........................... ok");

    return 0;
}

int dsi_reset(void)
{
    u32 val = 0;
    struct dsi_engine_registers* dsi = (struct dsi_engine_registers*)DSI_PROTOCOL_ENGINE_BASE;

    setv32(&val, SOFT_RESET, 1, 1);
    writel(val, &dsi->sysconfig);

    if(wait_for_bit(&dsi->sysstatus, 0, 1, 0, 5)) {
				dsserr("DSI softreset failed!");
				return 1;
    }

    dssmsg(" DSI reset is complete..................... ok");

    return 0;
}


void dsi_core_init(struct orion_display* d)
{
    u32 val = 0;

    setv32(&val, 0, 1, 1);       //enable automatic clock gating in the module
    setv32(&val, 2, 1, 1);       //Wakeup is enabled
    setv32(&val, 4, 2, 0x2);     //Smart-idle
    writel(val, &d->dctrl.dsi->sysconfig);
}

void dsi_initialize_irq(struct orion_display* d)
{
    writel(0, &d->dctrl.dsi->irqenable);             //disable all interrupts

    int i;
    u32 irq_channel_mask = 0xfffffff0;

    for(i = 0; i < 4; i++)
		writel(0, &d->dctrl.dsi->vc0_irqenable + 0x20 * i);

    writel(0, &d->dctrl.dsi->complexio_irqenable);
    writel(readl(&d->dctrl.dsi->irqstatus) & irq_channel_mask,
    	   &d->dctrl.dsi->irqstatus);     //clear interrupt status

    for(i = 0; i < 4; i++)
		writel(readl(&d->dctrl.dsi->vc0_irqstatus + 0x20 * i),
			&d->dctrl.dsi->vc0_irqstatus + 0x20 * i);

    writel(readl(&d->dctrl.dsi->complexio_irqstatus),
    	   &d->dctrl.dsi->complexio_irqstatus);
    writel(DSI_IRQ_ERROR_MASK | DSI_IRQ_TE_TRIGGER, &d->dctrl.dsi->irqenable);       // enable error irqs

    for(i = 0; i < 4; i++)
		writel(DSI_VC_IRQ_ERROR_MASK, &d->dctrl.dsi->vc0_irqenable + 0x20 * i);

    writel(DSI_CIO_IRQ_ERROR_MASK, &d->dctrl.dsi->complexio_irqenable);
}

void dsi_disable_irq(void)
{
    struct dsi_engine_registers* dsi = (struct dsi_engine_registers*)DSI_PROTOCOL_ENGINE_BASE;
    int i;

    writel(0, &dsi->irqenable);                                  //disable all dsi interrupts

    for(i = 0; i < 4; i++)
                writel(0, &dsi->vc0_irqenable + 0x20 * i);       //disable all vc interrupts

    writel(0, &dsi->complexio_irqenable);                        //disable all complexio interrupts
    writel(readl(&dsi->irqstatus), &dsi->irqstatus);              //clear interrupt status

    for(i = 0; i < 4; i++)
                writel(readl(&dsi->vc0_irqstatus + 0x20 * i),
                       &dsi->vc0_irqstatus + 0x20 * i);

    writel(readl(&dsi->complexio_irqstatus),
           &dsi->complexio_irqstatus);
}

int dsi_pll_init(struct orion_display* d)
{
//    struct display_controller_registers* dispc = (struct display_controller_registers*)DISPLAY_CONTROLLER_BASE;
//    struct dsi_engine_registers*         dsi   = (struct dsi_engine_registers*)DSI_PROTOCOL_ENGINE_BASE;
//    struct dsi_pll_registers*            pll   = (struct dsi_pll_registers*)DSI_PLL_BASE;

//     dispc_pck_free_enable
    r32setv(&d->dispc->control, 27, 1, 1);

    if(wait_for_bit(&d->dctrl.pll->status, 0, 1, 1, 100000)) {
				dsserr("PLL not coming out of reset!");
                                return 1;
    } //else
//    	dssmsg("PLL ok...");

//dispc_pck_free_disable
    r32setv(&d->dispc->control, 27, 1, 0);
//10    dsi_pll_power
    r32setv(&d->dctrl.dsi->clk_ctrl, 31, 2, 0x2);    //DSI_PLL_POWER_ON_ALL    = 0x2

    if(wait_for_bit(&d->dctrl.dsi->clk_ctrl, 29, 2, 0x2, 1000)) {
                                 dsserr("Failed to set DSI PLL power mode to POWER_ON_ALL!");
                                 return 1;
    } //else
//    	dssmsg("Set DSI PLL power mode to 2 ok...");

    dssmsg(" DSI PLL init done......................... ok");

    return 0;
}

unsigned int get_sys_clk(void)
{
    switch(readl(0x48306d40)) {
	case 0:
		return 12000000;
	case 1:
	        return 13000000;
	case 2:
	        return 19200000;
	case 3:
	        return 26000000;
	case 4:
	        return 38400000;
	case 5:
	        return 16800000;
    }
}

int dsi_configure_dsi_clocks(struct orion_display* d)
{
    u32 clkin, highfreq, regn, regm, clkin4ddr, regm3 = 0, regm4 = 0;

    if(d->clocks.fint < 32000 || d->clocks.fint > 52000000) {
		dsserr("wrong value of Fint!");
		return 1;
    }

    if(d->clocks.use_dss2_fck)
		clkin = get_sys_clk();//26000000;

    if(clkin < 32000000) {
		highfreq = 0;
		regn     = clkin/d->clocks.fint - 1;
    } else {
		highfreq = 1;
		regn     = clkin/2 * d->clocks.fint - 1;
    }

    clkin4ddr = d->timings.pixel_clock * (12 + (u32)d->color_depth * 4);//552
    regm      = (clkin4ddr * (regn + 1) * (highfreq + 1))/(2 * clkin/1000000);

    if(d->clocks.use_hsdiv) {
		regm3 = clkin4ddr/d->pixel_clk_div * d->timings.pixel_clock - 1;
		regm4 = clkin4ddr/d->pixel_clk_div * d->timings.pixel_clock - 1;
//		regm3 = clkin4ddr/120;//d->clocks.dsi1_pll_fclk - 1;
//              regm4 = clkin4ddr/120;//d->clocks.dsi2_pll_fclk - 1;
    }

    r32setv(&d->dctrl.pll->control, 0, 1, 0);        // DSI_PLL_AUTOMODE = manual
    u32 val = 0;//readl(&d->dctrl.pll->configuration1);
    setv32(&val, 0, 1, 1);				// DSI_PLL_STOPMODE
    setv32(&val, 7, 7,   regn);//0xc);
    setv32(&val, 18, 11, regm);//138);
    setv32(&val, 22, 4,  regm3 > 0 ? regm3 : 0);//5);
    setv32(&val, 26, 4,  regm4 > 0 ? regm4 : 0);//5);
    writel(val, &d->dctrl.pll->configuration1);

    val = 0;
    setv32(&val, 11, 1, d->clocks.use_dss2_fck ? 0 : 1);//0);                  //DSI_PLL_CLKSEL

//    u32 highfreq = d->clocks.use_dss2_fck ? 0 : 1;
    
    setv32(&val, 12, 1, highfreq);//0);                 //DSI_PLL_HIGHFREQ
    setv32(&val, 13, 1, 1);
    setv32(&val, 14, 1, 0);
    setv32(&val, 20, 1, 1);
    writel(val, &d->dctrl.pll->configuration2);
    writel(0x1, &d->dctrl.pll->go);

    if(wait_for_bit(&d->dctrl.pll->go, 0, 1, 0, 100000)) {
				dsserr("dsi pll go bit not going down!");
				return 1;
    }

    if(wait_for_bit(&d->dctrl.pll->status, 1, 1, 1, 100000)) {
				dsserr("cannot lock PLL!");
                                return 1;
    }

    val = (1 << 13) | (1 << 14) | (1 << 16) | (1 << 18);
    writel(val/*0x56000*/, &d->dctrl.pll->configuration2);	//This need after <wait_for_bit>

    dssmsg(" DSI clocks was configured................. ok");

    return 0;
}

int dss_select_dispc_clk_source(struct orion_display* d)//const int source)
{
//    struct display_subsystem_registers* dss = (struct display_subsystem_registers*)DISPLAY_SUBSYSTEM_BASE;
//    struct dsi_pll_registers*           pll = (struct dsi_pll_registers*)DSI_PLL_BASE;
//    int bit = source == DSS_SRC_DSS1_ALWON_FCLK ? 0 : 1;

//10.1  dsi_wait_dsi1_pll_active()
    if(wait_for_bit(&d->dctrl.pll->status, 7, 1, 1, 100000)) {
				dsserr("DSI1 PLL clock not active!");
                                return 1;
    } //else
//    	dssmsg("DSI1 PLL clock active...ok");
//10.1----------------------------------------------------------------
    r32setv(&d->dctrl.dss->control, 0, 1,
    	   (u32)(d->clocks.dispc_clk_src == DSS_SRC_DSS1_ALWON_FCLK ? 0 : 1));//bit);        //DISPC_CLK_SWITCH

    dssmsg(" DISPC clock source was selected........... ok");

    return 0;
}

int dss_select_dsi_clk_source(struct orion_display* d)//const int source)
{
    if(wait_for_bit(&d->dctrl.pll->status, 8, 1, 1, 100000)) {
				dsserr("DSI2 PLL clock not active!");
                                return 1;
    } //else
//    	dssmsg("DSI2 PLL clock active...ok");
    r32setv(&d->dctrl.dss->control, 1, 1,
    	   (u32)d->clocks.dsi_clk_src == DSS_SRC_DSS1_ALWON_FCLK ? 0 : 1);//bit);        //DSI_CLK_SWITCH

    dssmsg(" DSI clock source was selected............. ok");

    return 0;
}

void dsi_configure_dispc_clocks(struct orion_display* d)
{
    u32 val = 0;
    writel(setvalue(val, 7, 8, d->pixel_clk_div) | setvalue(val, 23, 8, d->logic_clk_div), &d->dispc->divisor);

    dssmsg(" DSI configure DISPC clocks is done........ ok");
}

int vc_enable(const int channel, bool enable)
{
    u32 *vc_ctrl = (u32*)(DSI_PROTOCOL_ENGINE_BASE + 0x100 + 0x20 * channel);

    r32setv(vc_ctrl, VC_EN, 1, (u32)enable);

    if(wait_for_bit(vc_ctrl, VC_EN, 1, (u32)enable, 100000)) {
#ifdef DSS_ERROR_OUTPUT
                    printf("ERROR: Failed to set dsi_vc%d_enable to %d!\n", channel, enable);
                    printf("DSI_VC%d_CTRL = 0x%x\n", channel, readl(vc_ctrl));
#endif
                    return 2;
    } //else
//	dssmsg("Enabled VC... ok");

    return 0;
}

int dsi_if_enable(bool enable)
{
    struct dsi_engine_registers* dsi = (struct dsi_engine_registers*)DSI_PROTOCOL_ENGINE_BASE;

    r32setv(&dsi->ctrl, IF_EN, 1, (u32)enable);

    if(wait_for_bit(&dsi->ctrl, IF_EN, 1, (u32)enable, 100000)) {
                            dsserr("Failed to set IF_EN to enable/disable");
                            printf("\tDSI_CTRL is 0x%x\n", readl(&dsi->ctrl));
                            return 1;
    } /*else {
                if(enable) dssmsg("The interface is enabled immediately... ok");
                else dssmsg("The interface is disabled... ok");
    }
*/
    return 0;
}

int dsi_complexio_init(struct orion_display* d)
{
//    struct dsi_engine_registers* dsi = (struct dsi_engine_registers*)DSI_PROTOCOL_ENGINE_BASE;
//    struct dsi_phy_registers*    phy = (struct dsi_phy_registers*)DSI_PHY_BASE;

    r32setv(&d->dctrl.dsi->clk_ctrl, 14, 1, 1);      // CIO_CLK_ICG, enable L3 clk to CIO
    readl(&d->dctrl.phy->register5);

    if(wait_for_bit(&d->dctrl.phy->register5, 30, 1, 1, 100000)) {
				dsserr("ComplexIO PHY not coming out of reset!");
                                printf("\tDSI_PHY_REGISTERr5 = 0x%x\n", readl(&d->dctrl.phy->register5));
                                return 1;
    } /*else {
            dssmsg("ComplexIO PHY reset done... ok\n\tReset done for the SCP clock domain.");

            if(readl(&phy->register5) & (1 << 29))
                        	dssmsg("Reset done for the PWR clock domain.");
    }
*/
//13.1  dsi_complexio_config

    u32 val = readl(&d->dctrl.dsi->complexio_cfg1);      //dsi.c   1429
    setv32(&val, 2, 3,  d->complexio.clk_lane);	//clk_lane = 2
    setv32(&val, 3, 1,  d->complexio.clk_pol);   // 0
    setv32(&val, 6, 3,  d->complexio.data1_lane);// 1
    setv32(&val, 7, 1,  d->complexio.data1_pol); // 0
    setv32(&val, 10, 3, d->complexio.data2_lane);// 3
    setv32(&val, 11, 1, d->complexio.data2_pol); // 0

    writel(val/*0x2a200312*/, &d->dctrl.dsi->complexio_cfg1);

//14.1  dsi_complexio_power(DSI_COMPLEXIO_POWER_ON);           1

    r32setv(&d->dctrl.dsi->complexio_cfg1, 28, 2, 1);        //state = 1

    if(wait_for_bit(&d->dctrl.dsi->complexio_cfg1, 26, 2, 1, 1000)) {
                            dsserr("failed to set complexio power state to 1!");
                            printf("\tDSI_COMPLEXIO_CFG1 = 0x%x\n", readl(&d->dctrl.dsi->complexio_cfg1));
                            return 1;
    } //else
//            dssmsg("complex I/O in ON state... ok");

    if(wait_for_bit(&d->dctrl.dsi->complexio_cfg1, 29, 1, 1, 100000)) {
                            dsserr("ComplexIO not coming out of reset!");
                            printf("\tDSI_COMPLEXIO_CFG1 = 0x%x\n", readl(&d->dctrl.dsi->complexio_cfg1));
                            return 1;
    } //else
//            dssmsg("Resetcomplex I/O completed... ok");

    if(wait_for_bit(&d->dctrl.dsi->complexio_cfg1, 21, 1, 1, 100000)) {
                            dsserr("ComplexIO LDO power supply is down!");
                            printf("\tDSI_COMPLEXIO_CFG1 = 0x%x\n", readl(&d->dctrl.dsi->complexio_cfg1));
                            return 1;
    } //else
//            dssmsg("ComplexIO LDO power supply is up... ok");

//15.1  dsi_complexio_timings()

    val = readl(&d->dctrl.phy->register0);
    setv32(&val, 31, 8, 0x0c);
    setv32(&val, 23, 8, 0x1b);
    setv32(&val, 15, 8, 0x0e);
    setv32(&val, 7, 8, 0x15);

    writel(val, &d->dctrl.phy->register0);   //0x0c1b0e15

    val = readl(&d->dctrl.phy->register1);
//      setv32(&val, 31, 8, 0x0c);
    setv32(&val, 22, 7, 0x04);
    setv32(&val, 15, 8, 0x0b);
    setv32(&val, 7, 8, 0x25);

    writel(val, &d->dctrl.phy->register1);   //0x42040b25

    val = readl(&d->dctrl.phy->register2);
    setv32(&val, 7, 8, 0x09);

    writel(val, &d->dctrl.phy->register2);   //0xb8000009

    if(dsi_if_enable(ENABLE))
                return 1;

    if(dsi_if_enable(DISABLE))
                return 1;

    r32setv(&d->dctrl.dsi->clk_ctrl, 20, 1, 1);

    if(dsi_if_enable(ENABLE))
                return 1;

    if(dsi_if_enable(DISABLE))
                return 1;

    dssmsg(" ComplexIO init done....................... ok");

    return 0;
}

void dsi_proto_timings(struct orion_display* d)
{
//    struct dsi_engine_registers* dsi = (struct dsi_engine_registers*)DSI_PROTOCOL_ENGINE_BASE;

    writel(0x0000130f, &d->dctrl.dsi->clk_timing);
    writel(0x000b000c, &d->dctrl.dsi->vm_timing7);
}

void dsi_set_lp_clk_divisor(struct orion_display* d)
{
//    struct dsi_engine_registers* dsi = (struct dsi_engine_registers*)DSI_PROTOCOL_ENGINE_BASE;

    r32setv(&d->dctrl.dsi->clk_ctrl, LP_CLK_DIVISOR, 13, 0x8);
    r32setv(&d->dctrl.dsi->clk_ctrl, LP_RX_SYNCHRO_ENABLE, 1, (u32)ENABLE);
}

void vc_initial_config(const int channel)
{
    u32 *vc_ctrl = (u32*)(DSI_PROTOCOL_ENGINE_BASE + 0x100 + 0x20 * channel);

    if(readl(vc_ctrl) & (1 << 15))
                printf("VC#%d is buzy when trying to configure it!\n", channel);

    u32 val = 0;

    setv32(&val, 1, 1, 0);
    setv32(&val, 2, 1, 0);
    setv32(&val, 3, 1, 0);
    setv32(&val, 4, 1, 0);
    setv32(&val, 7, 1, 1);
    setv32(&val, 8, 1, 1);
    setv32(&val, 9, 1, 0);
    setv32(&val, 29, 3, 0x4);
    setv32(&val, 23, 3, 0x4);

    vc_enable(channel, DISABLE);
    writel(val, vc_ctrl);
    vc_enable(channel, ENABLE);
}

void dsi_proto_config(void)
{
    struct dsi_engine_registers* dsi = (struct dsi_engine_registers*)DSI_PROTOCOL_ENGINE_BASE;

    writel(0x02020220, &dsi->tx_fifo_vc_size);      //void dsi_config_tx_fifo(enum ...
    writel(0x02020220, &dsi->rx_fifo_vc_size);      //void dsi_config_rx_fifo(enum ...
//dsi_set_stop_state_counter(0x1000, false, false);

    u32 val = readl(&dsi->timing1);
    setv32(&val, 15, 1, 0);
    setv32(&val, 14, 1, 0);
    setv32(&val, 13, 1, 0);
    setv32(&val, 12, 13, 0x1000);

    writel(val, &dsi->timing1);   //0xffff1000
//dsi_set_ta_timeout(0x1fff, true, true)

    val = readl(&dsi->timing1);
    setv32(&val, 31, 1, 1);
    setv32(&val, 30, 1, 1);
    setv32(&val, 29, 1, 1);
    setv32(&val, 28, 13, 0x1fff);

    writel(val, &dsi->timing1);   //
//dsi_set_lp_rx_timeout(0x1fff, true, true)

    val = readl(&dsi->timing2);
    setv32(&val, 15, 1, 1);
    setv32(&val, 14, 1, 1);
    setv32(&val, 13, 1, 1);
    setv32(&val, 12, 13, 0x1fff);

    writel(val, &dsi->timing2);
//dsi_set_hs_tx_timeout(0x1fff, true, true)

    val = readl(&dsi->timing2);
    setv32(&val, 31, 1, 1);
    setv32(&val, 30, 1, 1);
    setv32(&val, 29, 1, 1);
    setv32(&val, 28, 13, 0x1fff);

    writel(val, &dsi->timing2);   //

    val = readl(&dsi->ctrl);
    setv32(&val, 1, 1, 1);
    setv32(&val, 2, 1, 1);
    setv32(&val, 3, 1, 1);
    setv32(&val, 4, 1, 1);
    setv32(&val, 7, 2, 2);
    setv32(&val, 8, 1, 0);
    setv32(&val, 13, 2, 2);
    setv32(&val, 14, 1, 1);
    setv32(&val, 19, 1, 1);
    setv32(&val, 24, 1, 1);
    setv32(&val, 25, 1, 0);

    writel(val, &dsi->ctrl);

    int i;

    for(i = 0; i < 4; i++)
	    vc_initial_config(i);       //0x20800180
}

int init_dsi(struct orion_display* d)
{
    dsi_core_init(d);
    dsi_initialize_irq(d);

    if(dsi_pll_init(d))
				return 1;

    if(dsi_configure_dsi_clocks(d))
				return 1;

    if(dss_select_dispc_clk_source(d))//DSS_SRC_DSI1_PLL_FCLK))         //0
				return 1;

    if(dss_select_dsi_clk_source(d))//DSS_SRC_DSI2_PLL_FCLK))           //1
				return 1;

    dsi_configure_dispc_clocks(d);

    if(dsi_complexio_init(d)) {
            dsserr("don't complete dsi_complexio_init()");
            return 1;
    }

    dsi_proto_timings(d);
    dsi_set_lp_clk_divisor(d);
    dsi_proto_config();

    if(dsi_if_enable(ENABLE)) {
                dsserr("don't complete dsi_if_enable(ENABLE) in <dsi_if_enable>");
                return 1;
    }

    dssmsg(" DSI init done............................. ok");
//    if(dsi_force_tx_stop_mode_io())
//              puts(">error complete dsi_force_tx_stop_mode_io()\n");
    return 0;
}

/* here must be disabled all DSI interrupts  */
void uninit_dsi(void)
{
    dsi_disable_irq();
}

void enable_vc_irq(const int channel, const u32 irqtype, int enable)
{
    if(channel < 0 || channel > 3) {
                        dsserr("This channel number is not exist! <enable_vc_irq>");
                        return;
    }

    u32 *vc_irqenable = (u32*)(DSI_PROTOCOL_ENGINE_BASE + 0x11c + 0x20 * channel);

    enable = enable ? 1 : 0;

    r32setv(vc_irqenable, irqtype, 1, enable);
}

int dsi_force_tx_stop_mode_io(void)
{
    struct dsi_engine_registers* dsi = (struct dsi_engine_registers*)DSI_PROTOCOL_ENGINE_BASE;

    r32setv(&dsi->timing1, 15, 1, 1);

    if(wait_for_bit(&dsi->timing1, 15, 1, 0, 100000)) {
                        dsserr("TX_STOP bit not going down!");
                        printf("\tDSI_TIMING1 = 0x%x\n", readl(&dsi->timing1));
                        return 1;
    } //else
//        dssmsg("De-assertion of ForceTxStopMode... ok");

    return 0;
}

void vc_enable_hs_mode(const int channel, int enable)
{
    if(channel < 0 || channel > 3) {
                        dsserr("This channel number is not exist! <vc_enable_hs_mode>");
                        return;
    }

    u32 *vc_ctrl = (u32*)(DSI_PROTOCOL_ENGINE_BASE + 0x100 + 0x20 * channel);

    vc_enable(channel, DISABLE);
    dsi_if_enable(DISABLE);
    r32setv(vc_ctrl, MODE_SPEED, 1, enable ? 1 : 0);
    vc_enable(channel, ENABLE);
    dsi_if_enable(ENABLE);
    dsi_force_tx_stop_mode_io();
}

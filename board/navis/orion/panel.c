#include "timers.h"
#include "display.h"

#define VC0			0
#define SLEEP_OUT		0x11
#define DISPLAY_ON		0x29

DECLARE_GLOBAL_DATA_PTR;

void frame_reset(void)
{
    u32 *addr           = (u32 *)(gd->fb_base);
    u32 *end_frame_addr = addr + 480 * 800;

    while(addr < end_frame_addr)
		memset(addr++, 0, sizeof(addr));
}

void panel_backlight(int state)
{
    timer_t* tm = init_timer(gpt9);

    if(state) {
	config_timer(tm,
		    fast_pwm,
		    autoreload,
		    system_clk,
		    no_prescaler,
		    0xfffff5d7,
		    0xfffff800,
		    0xfffffff0);
	start_timer(tm);
    } else
	stop_timer(tm);
}

struct orion_display get_display_device(int device)
{
    struct orion_display d;

    if(device == 2) {
	d.dispc                = (struct display_controller_registers*)DISPLAY_CONTROLLER_BASE,
	d.dctrl.dsi	       = (struct dsi_engine_registers*)DSI_PROTOCOL_ENGINE_BASE,
	d.dctrl.dss	       = (struct display_subsystem_registers*)DISPLAY_SUBSYSTEM_BASE,
	d.dctrl.pll	       = (struct dsi_pll_registers*)DSI_PLL_BASE,
	d.dctrl.phy	       = (struct dsi_phy_registers*)DSI_PHY_BASE,
	d.timings.x_res        = 480,
	d.timings.y_res        = 800,
	d.timings.pixel_clock  = 30,//23,			//MHz
	d.timings.hsw          = 1,
	d.timings.hfp 	       = 1,
	d.timings.hbp 	       = 1,
	d.timings.vsw 	       = 1,
	d.timings.vfp 	       = 0,
	d.timings.vbp 	       = 0,
	d.clocks.use_dss2_fck  = true,
	d.clocks.use_hsdiv     = true,
	d.clocks.dispc_clk_src = DSS_SRC_DSI1_PLL_FCLK,
	d.clocks.dsi_clk_src   = DSS_SRC_DSI2_PLL_FCLK,
	d.clocks.fint          = 2000000,		//Hz
//	d.clocks.dsi1_pll_fclk = 96,//92,			//MHz
//	d.clocks.dsi2_pll_fclk = 96,//92,			//MHz
	d.color_depth	       = COLOR_DEPTH_24_BIT,
	d.display_type         = LCD_DISPLAY_TFT,
	d.interface_mode       = PARALLELMODE_DSI,
	d.fifohandcheck	       = ENABLE,
	d.logic_clk_div	       = 1,
	d.pixel_clk_div	       = 4,
	d.complexio.clk_lane   = 2,
	d.complexio.clk_pol    = 0,
	d.complexio.data1_lane = 1,
	d.complexio.data1_pol  = 0,
	d.complexio.data2_lane = 3,
	d.complexio.data2_pol  = 0;
    } else
    	memset(&d, 0, sizeof(d));

    return d;
}

#define ORION2		2
#define ORION3          3

int orion_display_enable(void)
{
    struct orion_display display =
    		get_display_device(ORION2);

    if(display.dispc == NULL) {
		dsserr("failed to start device!");
		return 1;
    }

    if(dss_reset()) {
                dsserr("DSS reset sequence is not completed");
		goto error;
    }

    if(init_dispc(&display)) {
//		uninit_dispc();
		dsserr("DISPC init failed!");
		goto error;
    }

    if(dsi_reset()) {
                dsserr("DSI softreset failed!");
		goto error;
    }

    if(init_dsi(&display)) {
                dsserr("DSI init failed!");
		goto error;
    }

    return 0;

error:
    dss_clocks(DISABLE);
    return 1;
}

int panel_init(void)
{
// common setting //
    unsigned char SETEXTC[4]  = {0xB9, 0xFF, 0x83, 0x69};
    unsigned char SETMIPI[14] = {
                        0xBA, 0x00, 0xA0, 0xC6,
                        0x00, 0x0A, 0x00, 0x10,
                        0x30, 0x6F, 0x02, 0x11,
                        0x18, 0x40,
    };
// command mode setting //
// Put 10ms after //
    unsigned char SETGIP[27] = {
                        0xD5, 0x00, 0x04, 0x03,
                        0x00, 0x01, 0x05, 0x28,
                        0x70, 0x01, 0x03, 0x00,
                        0x00, 0x40, 0x06, 0x51,
                        0x07, 0x00, 0x00, 0x41,
                        0x06, 0x50, 0x07, 0x07,
                        0x0F, 0x04, 0x00
    };

    unsigned char SETTPSNR[11] = {
            0xD8, 0x00, 0x12, 0x76,
            0xA7, 0x09, 0x67, 0x50,
            0x4E, 0x57, 0x75
    };

    if(orion_display_enable()) {
		dsserr("failed to enable display!");
		return 1;
    }

    enable_vc_irq(VC0, PACKET_SENT_IRQ, ENABLE);
    vc_enable_hs_mode(VC0, DISABLE);

    if(vc_dcs_write(VC0, SETEXTC, sizeof(SETEXTC)))
    						return 1;
    if(vc_dcs_write(VC0, SETGIP, sizeof(SETGIP)))
    						return 1;
    if(vc_dcs_write(VC0, SETTPSNR, sizeof(SETTPSNR)))
    						return 1;
    if(vc_dcs_write(VC0, SETMIPI, sizeof(SETMIPI)))
						return 1;

    u8 command = SLEEP_OUT;

    if(vc_dcs_write(VC0, &command, sizeof(command)))
    						return 1;
/*
    u8 id1, id2, id3;
    u8 buf[1];

    vc_dcs_read(0, 0xda, buf, 1);
    id1 = buf[0];

    vc_dcs_read(0, 0xdb, buf, 1);
    id2 = buf[0];

    vc_dcs_read(0, 0xdc, buf, 1);
    id3 = buf[0];

    printf(" ID1 = 0x%x\n ID2 = 0x%x\n ID3 = 0x%x\n", id1, id2, id3);
*/
    command = DISPLAY_ON;
    
    if(vc_dcs_write(VC0, &command, sizeof(command)))
    						return 1;

    enable_te(VC0, ENABLE);
    vc_enable_hs_mode(VC0, ENABLE);

    panel_backlight(ON);

    return 0;
}










static void set_color_mode(int mode)
{
    struct display_controller_registers* dispc = (struct display_controller_registers*)DISPLAY_CONTROLLER_BASE;

    r32setv(&dispc->gfx_attributes, 4, 4, (u32)mode);         //GFXFORMAT
}

static void set_plane(u32 paddr)
{
    struct display_controller_registers* dispc = (struct display_controller_registers*)DISPLAY_CONTROLLER_BASE;

    writel(paddr, &dispc->gfx_ba0);
    writel(paddr, &dispc->gfx_ba1);
}

static void configure_dispc(struct orion_graphics* g)
{
//    set_color_mode(0x8);
//    set_plane(0x8fc00000);



// setup_planes
// configure_dispc
// configure_overlay(OMAP_DSS_GFX = 0)
// enable_plane        dispc->gfx_attributes[0]
// dispc_set_color_mode(plane = 0, color_mode = 0x80)
    r32setv(&g->dispc->gfx_attributes, 4, 4, (u32)g->gfx_format);//0x8);         //GFXFORMAT
// dispc_set_plane_ba0(plane = 0, paddr(0x8fc00000) + offset0(0));
    writel(g->plane_addr, &g->dispc->gfx_ba0);                        //video_mem_addr
// dispc_set_plane_ba1(plane, paddr + offset1)
    writel(g->plane_addr, &g->dispc->gfx_ba1);
// dispc_set_row_inc(plane, row_inc = 1)
    writel(g->row_inc, &g->dispc->gfx_row_inc);
// dispc_set_pix_inc(plane, pix_inc = 1)
    writel(g->pix_inc, &g->dispc->gfx_pixel_inc);
// dispc_set_plane_pos(plane, pos_x = 0, pos_y = 0)
    r32setv(&g->dispc->gfx_position, 10, 11, g->gfx_position.x);
    r32setv(&g->dispc->gfx_position, 26, 11, g->gfx_position.y);
// dispc_set_pic_size(plane, width = 480, height = 800)
    r32setv(&g->dispc->gfx_size, 10, 11, g->pic_size.width  - 1);
    r32setv(&g->dispc->gfx_size, 26, 11, g->pic_size.height - 1);
// dispc_set_rotation_attrs(plane = 0, rotation = 0, mirror = 0, color_mode = 0x80)
    r32setv(&g->dispc->gfx_attributes, 13, 2, (u32)g->gfx_rotation);
    r32setv(&g->dispc->gfx_attributes,  8, 1, (u32)g->gfx_channel);
// if(plane != OMAP_DSS_VIDEO1) dispc_setup_global_alpha(plane, global_alpha = 0xff);
    writel((u32)g->global_alpha, &g->dispc->global_alpha);
// end of dispc_setup_plane ---------------------------------------------------------
// dispc_enable_replication(plane, c->replication = 0)    
    r32setv(&g->dispc->gfx_attributes, 5, 1, (u32)g->gfx_replication);
// dispc_set_burst_size(plane, c->burst_size = 2)
    r32setv(&g->dispc->gfx_attributes, 7, 2, (u32)g->burst_size);
// dispc_setup_plane_fifo(plane, c->fifo_low = 896, c->fifo_high = 960);
    r32setv(&g->dispc->gfx_fifo_threshold, 27, 12, g->fifo_threshold.low);
    r32setv(&g->dispc->gfx_fifo_threshold, 11, 12, g->fifo_threshold.high);
// dispc_enable_plane(plane, 1)

//    dispc_set_plane(ENABLE);

// end of configure_overlay(plane)      OMAP_DSS_GFX = 0 ---------------------------
// configure_manager(OMAP_DSS_CHANNEL_LCD = 0)
// dispc_set_default_color(channel, c->default_color = 0)
    if(g->gfx_channel == lcd)
	writel(g->default_color, &g->dispc->default_color_0);
    else
	writel(g->default_color, &g->dispc->default_color_1);
// dispc_set_trans_key(channel, c->trans_key_type, c->trans_key);

    if(g->gfx_channel == lcd) {
	r32setv(&g->dispc->config, 11, 1, 0);

	if((u32)g->gfx_format == 0x8)
		writel(0, &g->dispc->trans_color_0);
// dispc_enable_trans_key(channel, c->trans_enabled = 0);
	r32setv(&g->dispc->config, 10, 1, (u32)DISABLE);
// dispc_enable_alpha_blending(channel, c->alpha_enabled)
	r32setv(&g->dispc->config, 18, 1, (u32)DISABLE);
    } else {
	r32setv(&g->dispc->config, 13, 1, 0);

	if((u32)g->gfx_format == 0x8)
                writel(0, &g->dispc->trans_color_1);
    }
// end of configure_manager(channel) ----------------------------------------------  
// dispc_go(OMAP_DSS_CHANNEL_LCD = 0)

//    r32setv(&dispc->control, 9, 2, 0x3);
//    r32setv(&dispc->control, 0, 1, 1);                //enable lcd
//    r32setv(&dispc->control, 5, 1, 1);                //go lcd

// end of configure_dispc() -------------------------------------------------------
// dispc_set_lcd_size(*w, *h);
    r32setv(&g->dispc->size_lcd, 26, 11, g->pic_size.height - 1);
    r32setv(&g->dispc->size_lcd, 10, 11, g->pic_size.width  - 1);
}

static void dispc_set_plane(bool enable)
{
    struct display_controller_registers* dispc = (struct display_controller_registers*)DISPLAY_CONTROLLER_BASE;

    r32setv(&dispc->gfx_attributes, 0, 1, (u32)enable);
}

static int dispc_go(void)
{
    struct display_controller_registers* dispc = (struct display_controller_registers*)DISPLAY_CONTROLLER_BASE;

    r32setv(&dispc->control, 9, 2, 0x3);
    r32setv(&dispc->control, 0, 1, 1);          //enable lcd
    r32setv(&dispc->control, 3, 1, 1);
    wait_for_bit(&dispc->control, 5, 1, 0, 100000);
    r32setv(&dispc->control, 5, 1, 1);          //go lcd

    return 0;
}

int prepare_update(struct orion_graphics* g)
{
    configure_dispc(g);
    dispc_set_plane(ENABLE);
    dispc_go();

    return 0;
}

int send_update_window(struct orion_graphics* g)
{
    u16 x1 = (u16)g->gfx_position.x;
    u16 x2 = (u16)g->gfx_position.x + (u16)g->pic_size.width  - 1;
    u16 y1 = (u16)g->gfx_position.y;
    u16 y2 = (u16)g->gfx_position.y + (u16)g->pic_size.height - 1;

    u8 buf[5];
    buf[0] = DCS_COLUMN_ADDR;
    buf[1] = (x1 >> 8) & 0xff;
    buf[2] = (x1 >> 0) & 0xff;
    buf[3] = (x2 >> 8) & 0xff;
    buf[4] = (x2 >> 0) & 0xff;

    if(send_data_nosync(g->update_channel, buf, sizeof(buf)))
    					return 1;
    
    buf[0] = DCS_PAGE_ADDR;
    buf[1] = (y1 >> 8) & 0xff;
    buf[2] = (y1 >> 0) & 0xff;
    buf[3] = (y2 >> 8) & 0xff;
    buf[4] = (y2 >> 0) & 0xff;

    if(send_data_nosync(g->update_channel, buf, sizeof(buf)))
            				return 1;

    if(vc_send_bta(g->update_channel)) {
		dsserr("BTA was not received! <send_update_window>");
		return 1;
    }

    return 0;
}

#define VC_CTRL(c)		(u32*)(DSI_PROTOCOL_ENGINE_BASE + 0x100 + 0x20 * c)
#define VC_TE(c)		(u32*)(DSI_PROTOCOL_ENGINE_BASE + 0x104 + 0x20 * c)

int dsi_update_screen(struct orion_graphics* g)
{
    u32 *vc_ctrl = VC_CTRL(g->update_channel),
        *vc_te   = VC_TE(g->update_channel);
// dsi_update_screen_dispc(dssdev, x, y, w, h)
    unsigned pix_size = 24;
    unsigned bytespp = pix_size / 8;
    unsigned bytespl = g->pic_size.width * bytespp;
    unsigned bytespf = bytespl * g->pic_size.height;
    const unsigned line_buf_size = 1023 * 3;
    unsigned packet_payload;
    unsigned packet_len;
    unsigned total_len;

    if(bytespf < line_buf_size)
		packet_payload = bytespf;
    else
		packet_payload = (line_buf_size) / bytespl * bytespl;

    packet_len = packet_payload + 1;        /* 1 byte for DCS cmd */
    total_len = (bytespf / packet_payload) * packet_len;

    if(bytespf % packet_payload)
                    total_len += (bytespf % packet_payload) + 1;
    
    vc_config_vp(g->update_channel);
    r32setv(vc_te, 23, 24, total_len);            //TE_SIZE

    vc_write_long_header(g->update_channel, DSI_DT_DCS_LONG_WRITE, packet_len, 0);

    r32setv(vc_te, 31, 1, 1);                   //TE_START
    r32setv(&g->dispc->sysconfig, 4, 2, 1);
//    udelay(250000);                     //framedone_timeout
// start_update
// enable_channel(channel = OMAP_DSS_CHANNEL_LCD, enable)
// enable_lcd_out(int enable)
    r32setv(&g->dispc->control, 0, 1, (u32)ENABLE);
//    udelay(100000);             //waiting for FRAME DONE
// void _omap_dispc_set_irqs(void)
    r32setv(&g->dispc->irqenable, 0, 1, 1);
// end of enable_lcd_out(int enable) ------------------------------- 
// end of enable_channel() ------------------------------------------
// end of start_update ----------------------------------------------
    r32setv(&g->dsi->timing2, 15, 1, (u32)DISABLE);
//    vc_send_bta_nosync(channel);
    r32setv(vc_ctrl, 6, 1, 1);           //send BTA
//    udelay(250000);
// end of dsi_update_screen_dispc(dssdev, x, y, w, h) ---------------
    return 0;
}

struct orion_graphics get_graphics_device(int device)
{
    struct orion_graphics g;

    if(device == 2 || device == 3) {
	g.dispc = (struct display_controller_registers*)DISPLAY_CONTROLLER_BASE,
	g.dsi   = (struct dsi_engine_registers*)DSI_PROTOCOL_ENGINE_BASE;
	g.gfx_format          = RGB24;
	g.plane_addr          = gd->fb_base;
	g.row_inc             = 1;
	g.pix_inc             = 1;
	g.gfx_position.x      = 0;
	g.gfx_position.y      = 0;
	g.pic_size.width      = 480;
	g.pic_size.height     = 800;
	g.gfx_rotation        = none;
	g.gfx_channel         = lcd;
	g.global_alpha        = 0xff;
	g.gfx_replication     = false;
	g.burst_size          = bursts_16x32bit;
	g.fifo_threshold.high = 960;
	g.fifo_threshold.low  = 896;
	g.default_color       = 0;
	g.update_channel      = VC0;
    } else
	memset(&g, 0, sizeof(g));

    return g;
}

int panel_update(void)
{
    struct orion_graphics graphics =
                    get_graphics_device(ORION2);

    prepare_update(&graphics);
    
    if(send_update_window(&graphics))
		return 1;

    dsi_update_screen(&graphics);

    return 0;
}















#include "timers.h"
#include "display.h"

#define START_VMEM_ADDR         0x8fc00000
#define VC0			0
#define SLEEP_OUT		0x11
#define DISPLAY_ON		0x29

void frame_reset(void)
{
    u32 *addr           = (u32 *)(START_VMEM_ADDR);
    u32 *end_frame_addr = addr + 480 * 800;

    while(addr < end_frame_addr) {
      /*  *((ulong*)addr) = (ulong)0; */
        *((ulong*)addr) ^= *((ulong*)addr);
      /*  addr += sizeof(ulong);*/
      addr++;
    }
}
/*
void set_line(const int x, int line, int* points, int len)
{
    int offset,
        y,
        dot,
        i;
    u32 addr, color;

    for(i = 0; i < len; i++) {
            if(points[i])
                color = 0xffffffff;
            else
                color = 0;

                y = 360 * (line - 1) + 1;
                dot = y + x + i;
                offset = 4 * (dot - 1);
                addr = 0x8fc00000 + offset;
                *((ulong*)addr) = (ulong)color;
    }
}

void char_to_frame(const int x, int line, int* chr, int str_count, int pix_count)
{
    int i, *p;

    for(i = 0; i < str_count; i++) {
        p = &chr[i * pix_count];
        set_line(x, line++, p, pix_count);

        if(str_count == 6)
                set_line(x, line++, p, pix_count);
    }
}
*/
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

int orion_display_enable(void)
{
    if(dss_reset()) {
                dsserr("DSS reset sequence is not completed");
                return 1;
    } //else
//      dssmsg("DSS reset sequence is complete... ok");

//    display_init_dispc();
    init_dispc();

    if(dsi_reset()) {
                dsserr("DSI softreset failed!");
                return 1;
    } //else
//      dssmsg("The softreset DSI completed... ok");

    if(display_init_dsi()) {
                dsserr("DSI init failed!");
                return 1;
    }

    return 0;
}

int panel_init(void)
{
// common setting //
    unsigned char SETEXTC[4] = {0xB9, 0xFF, 0x83, 0x69};
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

    orion_display_enable();
/*
    if(dss_reset()) {
                dsserr("DSS reset sequence is not completed");
                return 1;
    } //else
//	dssmsg("DSS reset sequence is complete... ok");

//    display_init_dispc();
    init_dispc();

    if(dsi_reset()) {
                dsserr("DSI softreset failed!");
                return 1;
    } //else
//	dssmsg("The softreset DSI completed... ok");

    if(display_init_dsi()) {
		dsserr("DSI init failed!");
                return 1;
    }
*/
    enable_vc_irq(VC0, PACKET_SENT_IRQ, ENABLE);
    vc_enable_hs_mode(VC0, DISABLE);

    if(vc_dcs_write(VC0, SETEXTC, sizeof(SETEXTC)))
    						return 1;
    if(vc_dcs_write(VC0, SETGIP, sizeof(SETGIP)))
    						return 1;
//    udelay(10000),
    if(vc_dcs_write(VC0, SETTPSNR, sizeof(SETTPSNR)))
    						return 1;
    if(vc_dcs_write(VC0, SETMIPI, sizeof(SETMIPI)))
						return 1;

    u8 command = SLEEP_OUT;

    if(vc_dcs_write(VC0, &command, sizeof(command)))
    						return 1;
//    udelay(120000);
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

int prepare_update(void)
{
    struct display_controller_registers* dispc = (struct display_controller_registers*)DISPLAY_CONTROLLER_BASE;

// setup_planes
// configure_dispc
// configure_overlay(OMAP_DSS_GFX = 0)
// enable_plane        dispc->gfx_attributes[0]
// dispc_set_color_mode(plane = 0, color_mode = 0x80)
    r32setv(&dispc->gfx_attributes, 4, 4, 0x9);		//GFXFORMAT
// dispc_set_plane_ba0(plane = 0, paddr(0x8fc00000) + offset0(0));
    writel(0x8fc00000, &dispc->gfx_ba0);			//video_mem_addr
// dispc_set_plane_ba1(plane, paddr + offset1)
    writel(0x8fc00000, &dispc->gfx_ba1);
// dispc_set_row_inc(plane, row_inc = 1)
    writel(0x1, &dispc->gfx_row_inc);
// dispc_set_pix_inc(plane, pix_inc = 1)
    writel(0x1, &dispc->gfx_pixel_inc);
// dispc_set_plane_pos(plane, pos_x = 0, pos_y = 0)
    r32setv(&dispc->gfx_position, 10, 11, 0);
    r32setv(&dispc->gfx_position, 26, 11, 0);
// dispc_set_pic_size(plane, width = 480, height = 800)
    r32setv(&dispc->gfx_size, 10, 11, 480 - 1);
    r32setv(&dispc->gfx_size, 26, 11, 800 - 1);
// dispc_set_rotation_attrs(plane = 0, rotation = 0, mirror = 0, color_mode = 0x80)
    r32setv(&dispc->gfx_attributes, 13, 2, 0);
    r32setv(&dispc->gfx_attributes, 18, 1, 0);
// if(plane != OMAP_DSS_VIDEO1) dispc_setup_global_alpha(plane, global_alpha = 0xff);
    writel(0xff, &dispc->global_alpha);
// end of dispc_setup_plane ---------------------------------------------------------
// dispc_enable_replication(plane, c->replication = 0)    
    r32setv(&dispc->gfx_attributes, 5, 1, 0);
// dispc_set_burst_size(plane, c->burst_size = 2)
    r32setv(&dispc->gfx_attributes, 7, 2, 0x2);
// dispc_setup_plane_fifo(plane, c->fifo_low = 896, c->fifo_high = 960);
    r32setv(&dispc->gfx_fifo_threshold, 27, 12, 896);
    r32setv(&dispc->gfx_fifo_threshold, 11, 12, 960);
// dispc_enable_plane(plane, 1)
    r32setv(&dispc->gfx_attributes, 0, 1, 1);
// end of configure_overlay(plane) 	OMAP_DSS_GFX = 0 ---------------------------
// configure_manager(OMAP_DSS_CHANNEL_LCD = 0)
// dispc_set_default_color(channel, c->default_color = 0)
    writel(0, &dispc->default_color_0);
// dispc_set_trans_key(channel, c->trans_key_type, c->trans_key);
    r32setv(&dispc->config, 11, 1, 0);
    writel(0, &dispc->trans_color_0);
// dispc_enable_trans_key(channel, c->trans_enabled = 0);
    r32setv(&dispc->config, 10, 1, 0);
// dispc_enable_alpha_blending(channel, c->alpha_enabled)
    r32setv(&dispc->config, 18, 1, 0);
// end of configure_manager(channel) ----------------------------------------------  
// dispc_go(OMAP_DSS_CHANNEL_LCD = 0)
    r32setv(&dispc->control, 0, 1, 1);		//enable lcd
    r32setv(&dispc->control, 5, 1, 1);		//go lcd
// end of configure_dispc() -------------------------------------------------------
// dispc_set_lcd_size(*w, *h);
    r32setv(&dispc->size_lcd, 26, 11, 800 - 1);
    r32setv(&dispc->size_lcd, 10, 11, 480 - 1);



// taal_set_update_window(x = 0, y = 0, w = 480, h = 800)


// end of taal_set_update_window(x, y, w, h) -------------------------------------


    return 0;
}

int send_update_window(u16 x, u16 y, u16 w, u16 h)
{
    u16 x1 = x;
    u16 x2 = x + w - 1;
    u16 y1 = y;
    u16 y2 = y + h - 1;

    u8 buf[5];
    buf[0] = DCS_COLUMN_ADDR;
    buf[1] = (x1 >> 8) & 0xff;
    buf[2] = (x1 >> 0) & 0xff;
    buf[3] = (x2 >> 8) & 0xff;
    buf[4] = (x2 >> 0) & 0xff;

    if(send_data_nosync(VC0, buf, sizeof(buf)))
    					return 1;
    
    buf[0] = DCS_PAGE_ADDR;
    buf[1] = (y1 >> 8) & 0xff;
    buf[2] = (y1 >> 0) & 0xff;
    buf[3] = (y2 >> 8) & 0xff;
    buf[4] = (y2 >> 0) & 0xff;

    if(send_data_nosync(VC0, buf, sizeof(buf)))
            				return 1;

    if(vc_send_bta(VC0)) {
		dsserr("BTA was not received! <send_update_window>");
		return 1;
    }

    return 0;
}

int dsi_update(const int channel)
{

    struct display_controller_registers* dispc = (struct display_controller_registers*)DISPLAY_CONTROLLER_BASE;
    struct dsi_engine_registers*         dsi   = (struct dsi_engine_registers*)DSI_PROTOCOL_ENGINE_BASE;
    u32 *vc_ctrl                = (u32*)(DSI_PROTOCOL_ENGINE_BASE + 0x100 + 0x20 * channel),
        *vc_te                  = (u32*)(DSI_PROTOCOL_ENGINE_BASE + 0x104 + 0x20 * channel);
// dsi_update_screen_dispc(dssdev, x, y, w, h)
    vc_config_vp(VC0);
    r32setv(vc_te, 23, 24, 1152400);            //TE_SIZE

    vc_write_long_header(VC0, DSI_DT_DCS_LONG_WRITE, /*packet_len*/ 2881, 0);

    r32setv(vc_te, 31, 1, 1);                   //TE_START
    r32setv(&dispc->sysconfig, 4, 2, 1);
//    udelay(250000);                     //framedone_timeout
// start_update
// enable_channel(channel = OMAP_DSS_CHANNEL_LCD, enable)
// enable_lcd_out(int enable)
    r32setv(&dispc->control, 0, 1, 1/*enable*/);
//    udelay(100000);             //waiting for FRAME DONE
// void _omap_dispc_set_irqs(void)
    r32setv(&dispc->irqenable, 0, 1, 1);
// end of enable_lcd_out(int enable) ------------------------------- 
// end of enable_channel() ------------------------------------------
// end of start_update ----------------------------------------------
    r32setv(&dsi->timing2, 15, 1, 0);
//    vc_send_bta_nosync(0);
    r32setv(vc_ctrl, 6, 1, 1);           //send BTA
//    udelay(250000);
// end of dsi_update_screen_dispc(dssdev, x, y, w, h) ---------------
    return 0;
}

int panel_update(void)
{
    prepare_update();
    send_update_window(0, 0, 480, 800);
    dsi_update(VC0);
}















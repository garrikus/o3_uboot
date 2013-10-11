#include <asm/io.h>
#include <asm/arch-omap3/sys_proto.h>
#include "display.h"


int vc_enable(const int, int);
int wait_for_bit(volatile void*, u8, u8, u32, u32);
void dsserr(const char*);
void dssmsg(const char*);
inline void r32setv(void* rg, u8 offset, u8 count, u32 data);
inline void setv32(u32* source, u8 offset, u8 count, u32 data);
void putc(char);
void puts(const char*);
void udelay(unsigned long);
void printf(const char*, ...);


int vc_config_vp(const int channel)
{
    u32* vc_ctrl = (u32*)(DSI_PROTOCOL_ENGINE_BASE + 0x100 + 0x20 * channel);

    vc_enable(channel, DISABLE);

    if(wait_for_bit(vc_ctrl, 15, 1, 0, 100000)) {
					dsserr("VC is BUZY!\n");
					return 1;
    } //else
//	dssmsg("VC is not BUZY... ok");

    r32setv(vc_ctrl, SOURCE, 1, 1);                                  // source is video port
    vc_enable(channel, ENABLE);

    return 0;
}

int vc_config_l4(const int channel)
{
    u32 *vc_ctrl = (u32*)(DSI_PROTOCOL_ENGINE_BASE + 0x100 + 0x20 * channel);

    vc_enable(channel, DISABLE);

    if(wait_for_bit(vc_ctrl, 15, 1, 0, 100000)) {
                                        dsserr("VC is BUZY!");
                                        return 1;
    } //else
//                dssmsg("VC is not BUZY... ok");

    r32setv(vc_ctrl, SOURCE, 1, 0);                                  // source is l4
    vc_enable(channel, ENABLE);

    return 0;
}

void vc_write_long_header(const int channel, const u8 data_type, u16 len, u8 ecc)
{
    u32* vc_long_packet_header = (u32*)(DSI_PROTOCOL_ENGINE_BASE + 0x108 + 0x20 * channel);
    u8 data_id = data_type | (channel << 6);            //prepare packet
    u32 val = 0;

    setv32(&val, 7, 8, data_id);
    setv32(&val, 23, 16, len);
    setv32(&val, 31, 8, ecc);
    writel(val, vc_long_packet_header);
}

void vc_write_long_payload(const int channel, u8 b1, u8 b2, u8 b3, u8 b4)
{
    u32* vc_long_packet_payload = (u32*)(DSI_PROTOCOL_ENGINE_BASE + 0x10c + 0x20 * channel);
    u32 val = b4 << 24 | b3 << 16 | b2 << 8  | b1 << 0;

    r32setv(vc_long_packet_payload, 31, 32, val);
}

int is_packet_sent(const int channel)
{
    u32* vc_irqstatus = (u32*)(DSI_PROTOCOL_ENGINE_BASE + 0x118 + 0x20 * channel);

    if(wait_for_bit(vc_irqstatus, 2, 1, 1, 100000)) {
				dsserr("a packet has not been sent!");
				return 1;
    } //else
//	dssmsg("a packet has been sent... ok");

    return 0;
}


int vc_send_long(const int channel, const u8 data_type, u8 *data, u16 len, u8 ecc)
{
    dssmsg("<sending long>");

    vc_config_l4(channel);
    vc_write_long_header(channel, data_type, len, ecc);

    u8* p = data;
    u8 b1, b2, b3, b4;
    int i;

    for(i = 0; i < len >> 2; i++) {
#ifdef DSS_DEBUG_OUTPUT
//        if(!i)
            printf("\tsending full packet %d\n", i);
#endif
        b1 = *p++;
        b2 = *p++;
        b3 = *p++;
        b4 = *p++;

        vc_write_long_payload(channel, b1, b2, b3, b4);
    }

    i = len % 4;

    if(i) {
        b1 = 0; b2 = 0; b3 = 0;
#ifdef DSS_DEBUG_OUTPUT
//        if(0 && dsi.debug_write)
                        printf("\tsending remainder bytes %d\n", i);
#endif
        switch(i) {
            case 3:
                    b1 = *p++;
                    b2 = *p++;
                    b3 = *p++;
                    break;
            case 2:
                    b1 = *p++;
                    b2 = *p++;
                    break;
            case 1:
                    b1 = *p++;
                    break;
        }

        vc_write_long_payload(channel, b1, b2, b3, 0);
    }

    return is_packet_sent(channel);
}

int vc_send_short(const int channel, u8 data_type, u16 data, u8 ecc)
{
    dssmsg("<sending short>");

    u32 *vc_ctrl                = (u32*)(DSI_PROTOCOL_ENGINE_BASE + 0x100 + 0x20 * channel),
        *vc_short_packet_header = (u32*)(DSI_PROTOCOL_ENGINE_BASE + 0x110 + 0x20 * channel);

    vc_config_l4(channel);

    if(readl(vc_ctrl) & (1 << 16)) {
                        dsserr("The TX FIFO is full!");
                        return 2;
    }

    writel((((data_type | (channel << 6)) << 0) | (data << 8) | (ecc << 24)), vc_short_packet_header);

    return is_packet_sent(channel);
}

int send_data_nosync(const int channel, u8* data, u8 len)
{
    if(len == 1) {
                return vc_send_short(channel, DSI_DT_DCS_SHORT_WRITE_0, data[0], 0);
    } else if(len == 2) {
                return vc_send_short(channel, DSI_DT_DCS_SHORT_WRITE_1, data[0] | (data[1] << 8), 0);
    } else {
                return vc_send_long(channel, DSI_DT_DCS_LONG_WRITE, data, len, 0);      //DSI_DT_DCS_LONG_WRITE = 0x39
    }
}

void vc_enable_bta_irq(const int channel, int enable)
{
    if(channel < 0 || channel > 3) {
			dsserr("This channel number is not exist! <vc_enable_bta_irq>");
			return;
    }

    u32 *vc_irqstatus = (u32*)(DSI_PROTOCOL_ENGINE_BASE + 0x118 + 0x20 * channel),
	*vc_irqenable = (u32*)(DSI_PROTOCOL_ENGINE_BASE + 0x11c + 0x20 * channel);
    
    enable = enable ? 1 : 0;

//    r32setv(vc_ctrl, 0, 1, 0);                  //disable VC
    vc_enable(channel, DISABLE);
    r32setv(vc_irqstatus, 5, 1, 1);             //reset BTA IRQ
    r32setv(vc_irqenable, 5, 1, enable);        //enable irq bta
//    r32setv(vc_ctrl, 0, 1, 1);                  //enable VC
    vc_enable(channel, ENABLE);
}

int wait_for_completion_bta_timeout(const int channel)
{
    if(channel < 0 || channel > 3) {
			dsserr("This channel number is not exist! <wait_for_completion_bta_timeout>");
			return 1;
    }

    struct dsi_engine_registers* dsi = (struct dsi_engine_registers*)DSI_PROTOCOL_ENGINE_BASE;
    u32* vc_irqstatus                = (u32*)(DSI_PROTOCOL_ENGINE_BASE + 0x118 + 0x20 * channel);

    u32 irqstatus, vcstatus;
    int t = 100000;

    while(t--) {
                irqstatus = readl(&dsi->irqstatus),
                vcstatus  = readl(vc_irqstatus);

                if(irqstatus & 1)
                        if(vcstatus & (1 << 5)) {
//                                dssmsg("BTA IRQ was received... ok");
                                writel(vcstatus, vc_irqstatus);
                                readl(vc_irqstatus);
                                return 0;
                        }	

                udelay(1);
    }

    return 1;
}

int vc_send_bta(const int channel)
{
    if(channel < 0 || channel > 3) {
			dsserr("This channel number is not exist!");
			return 1;
    }

    u32 *vc_ctrl                = (u32*)(DSI_PROTOCOL_ENGINE_BASE + 0x100 + 0x20 * channel),
    	*vc_irqstatus           = (u32*)(DSI_PROTOCOL_ENGINE_BASE + 0x118 + 0x20 * channel),
        *vc_short_packet_header = (u32*)(DSI_PROTOCOL_ENGINE_BASE + 0x110 + 0x20 * channel);

    if(readl(vc_ctrl) & (1 << 20))
    {
        puts("WARNING: The RX FIFO is not empty!\n");
/*
//////dsi_vc_flush_receive_data(channel)
        while(readl(vc_ctrl) & (1 << 20))
        {
            val = readl(vc_short_packet_header);
            a = (1 << 6);
            a--;
            a &= (u8)val;

            if(a == 0x2) puts("\tlook the 'dsi_show_rx_ack_with_err'!\n");  //dsi.c 1907
            else if(a == 0x21) printf("\tDCS short response, 1 byte: %#x\n", (u16)((val & 0xffff00) >> 8));
            else if(a == 0x22) printf("\tDCS short response, 2 byte: %#x\n", (u16)((val & 0xffff00) >> 8));
            else if(a == 0x1c) {
                        printf("\tDCS long response, len %d\n", (u16)((val & 0xffff00) >> 8));
////////dsi_vc_flush_long_data(channel)
                        while(readl(vc_ctrl) & (1 << 20))
                        {
                            val = readl(vc_short_packet_header);
                            printf("\t\tb1 %#02x b2 %#02x b3 %#02x b4 %#02x\n",
                                (val >> 0) & 0xff,
                                (val >> 8) & 0xff,
                                (val >> 16) & 0xff,
                                (val >> 24) & 0xff);
                        }
            } else printf("\tunknown datatype 0x%02x\n", a);
        }
//////-------------------------------------------------------------
    return 1;
*/
    }

    dssmsg("<send BTA signal>");

    vc_enable_bta_irq(channel, ENABLE);

    if(readl(vc_irqstatus) & (1 << 5)) {
                                    dsserr("BTA IRQ is set!");
//                                      r32setv(&dsi->vc0_irqstatus, 5, 1, 1);
    }// else puts(" BTA irq is not set\n");

    if(readl(vc_ctrl) & (1 << 20)) dsserr("The RX FIFO is not empty!");

    r32setv(vc_ctrl, 6, 1, 1);           //send BTA

    if(wait_for_bit(vc_ctrl, 6, 1, 0, 100000)) {
                                    dsserr("BTA generation was not completed!");
                                    return 1;
    } //else
//      dssmsg("BTA generation is completed... ok");

    udelay(500);

    if(wait_for_completion_bta_timeout(channel)) {
//                             dsserr("BTA was not received!");
                             return 1;
    }

    return 0;
}

int vc_dcs_write(const int channel, u8* data, u8 len)
{
    if(send_data_nosync(channel, data, len)) {
                        dsserr(/*error code*/"Data was not sent!");
                        return 1;
    }

    if(vc_send_bta(channel)) {
                        dsserr("BTA was not received!");
                        return 1;
    }
/*
    if(readl(&dsi->vc0_ctrl) & (1 << 20)) {
                        dsserr("<vc_dcs_write> RX FIFO not empty after write, dumping data:");
//                      vc_flush_receive_data(channel);
                        return 1;
    }
*/
    return 0;
}

int vc_dcs_read(const int channel, u8 dcs_cmd, u8 *buf, int buflen)
{
    u32 val;
    u8 dt;
    int r = 1;
    u32 *vc_ctrl                = (u32*)(DSI_PROTOCOL_ENGINE_BASE + 0x100 + 0x20 * channel),
	*vc_short_packet_header = (u32*)(DSI_PROTOCOL_ENGINE_BASE + 0x110 + 0x20 * channel);

    if(vc_send_short(channel, DSI_DT_DCS_READ, dcs_cmd, 0))
                                                            goto err;

//    r = dsi_vc_send_bta_sync(channel);
    if(vc_send_bta(channel)) {
                puts("ERROR: Failed to receive BTA!\n");
                goto err;
    } //else
//        dssmsg("display_on BTA response is received... ok");

        /* RX_FIFO_NOT_EMPTY */
    if(!(readl(vc_ctrl) & (1 << 20))) {
            puts("RX fifo empty when trying to read.\n");
            goto err;
    }

    val = readl(vc_short_packet_header);

    printf("\theader: %08x\n", val);
//    dt = FLD_GET(val, 5, 0);
    dt = val & 0x3f;

    if(dt == DSI_DT_RX_ACK_WITH_ERR) {
//            u16 err = FLD_GET(val, 23, 8);
//            dsi_show_rx_ack_with_err(err);
            puts("ERROR: dt = 0x02\n");
            goto err;
    } else if(dt == DSI_DT_RX_SHORT_READ_1) {
                u8 data = val & 0xff00;
                data >>= 8;
                printf("\tDCS short response, 1 byte: %02x\n", data);

                if (buflen < 1) {
                        goto err;
                }

                buf[0] = data;

                return 1;
        } else if (dt == DSI_DT_RX_SHORT_READ_2) {
                u16 data = val & 0xffff00;
                data >>= 8;
                printf("\tDCS short response, 2 byte: %04x\n", data);

                if (buflen < 2) {
                        goto err;
                }

                buf[0] = data & 0xff;
                buf[1] = (data >> 8) & 0xff;

                return 2;
        } else if (dt == DSI_DT_RX_DCS_LONG_READ) {
                int w;
                int len = val & 0xffff00;
                len >>= 8;
                printf("\tDCS long response, len %d\n", len);

                if (len > buflen) {
                        goto err;
                }

                /* two byte checksum ends the packet, not included in len */
                for (w = 0; w < len + 2;) {
                        int b;
                        val = readl(vc_short_packet_header);
                        printf("\t\t%02x %02x %02x %02x\n",
                                                (val >> 0) & 0xff,
                                                (val >> 8) & 0xff,
                                                (val >> 16) & 0xff,
                                                (val >> 24) & 0xff);

                        for (b = 0; b < 4; ++b) {
                                if (w < len)
                                        buf[w] = (val >> (b * 8)) & 0xff;
                                /* we discard the 2 byte checksum */
                                ++w;
                        }
                }

                return len;
        } else {
                printf("\tunknown datatype 0x%02x\n", dt);
                goto err;
        }

err:
        printf("dsi_vc_dcs_read(ch %d, cmd 0x%02x) failed\n", channel, dcs_cmd);
        return r;
}

int enable_te(const int channel, int enable)
{
	if(enable) {
		u8 buf[2] = {
	       		DCS_TEAR_ON,
	        	0
		};

		return vc_dcs_write(channel, buf, 2);
	} else {
		u8 cmd = DCS_TEAR_OFF;
		
		return vc_dcs_write(channel, &cmd, 1);
	}
}



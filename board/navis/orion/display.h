#ifndef __DSS_DRIVER_H__
#define __DSS_DRIVER_H__


#define ENABLE				1
#define DISABLE				0
#define ON				1
#define OFF				0


/* Global interrupts */
#define DSI_IRQ_HS_TX_TIMEOUT           (1 << 14)
#define DSI_IRQ_LP_RX_TIMEOUT           (1 << 15)
#define DSI_IRQ_SYNC_LOST               (1 << 18)
#define DSI_IRQ_TA_TIMEOUT              (1 << 20)

#define DSI_IRQ_ERROR_MASK              (DSI_IRQ_HS_TX_TIMEOUT | \
                                        DSI_IRQ_LP_RX_TIMEOUT  | \
                                        DSI_IRQ_SYNC_LOST      | \
                                        DSI_IRQ_TA_TIMEOUT)

#define DSI_IRQ_TE_TRIGGER              (1 << 16)

/* Virtual channel interrupts */
#define DSI_VC_IRQ_CS                   (1 << 0)
#define DSI_VC_IRQ_ECC_CORR             (1 << 1)
#define DSI_VC_IRQ_PACKET_SENT          (1 << 2)
#define DSI_VC_IRQ_FIFO_TX_OVF          (1 << 3)
#define DSI_VC_IRQ_FIFO_RX_OVF          (1 << 4)
#define DSI_VC_IRQ_BTA                  (1 << 5)
#define DSI_VC_IRQ_ECC_NO_CORR          (1 << 6)
#define DSI_VC_IRQ_FIFO_TX_UDF          (1 << 7)
#define DSI_VC_IRQ_PP_BUSY_CHANGE       (1 << 8)

#define DSI_VC_IRQ_ERROR_MASK           (DSI_VC_IRQ_CS         | \
                                        DSI_VC_IRQ_ECC_CORR    | \
                                        DSI_VC_IRQ_FIFO_TX_OVF | \
                                        DSI_VC_IRQ_FIFO_RX_OVF | \
                                        DSI_VC_IRQ_ECC_NO_CORR | \
                                        DSI_VC_IRQ_FIFO_TX_UDF)

/* ComplexIO interrupts */
#define DSI_CIO_IRQ_ERRSYNCESC1         (1 << 0)
#define DSI_CIO_IRQ_ERRSYNCESC2         (1 << 1)
#define DSI_CIO_IRQ_ERRSYNCESC3         (1 << 2)
#define DSI_CIO_IRQ_ERRESC1             (1 << 5)
#define DSI_CIO_IRQ_ERRESC2             (1 << 6)
#define DSI_CIO_IRQ_ERRESC3             (1 << 7)
#define DSI_CIO_IRQ_ERRCONTROL1         (1 << 10)
#define DSI_CIO_IRQ_ERRCONTROL2         (1 << 11)
#define DSI_CIO_IRQ_ERRCONTROL3         (1 << 12)
#define DSI_CIO_IRQ_STATEULPS1          (1 << 15)
#define DSI_CIO_IRQ_STATEULPS2          (1 << 16)
#define DSI_CIO_IRQ_STATEULPS3          (1 << 17)
#define DSI_CIO_IRQ_ERRCONTENTIONLP0_1  (1 << 20)
#define DSI_CIO_IRQ_ERRCONTENTIONLP1_1  (1 << 21)
#define DSI_CIO_IRQ_ERRCONTENTIONLP0_2  (1 << 22)
#define DSI_CIO_IRQ_ERRCONTENTIONLP1_2  (1 << 23)
#define DSI_CIO_IRQ_ERRCONTENTIONLP0_3  (1 << 24)
#define DSI_CIO_IRQ_ERRCONTENTIONLP1_3  (1 << 25)
#define DSI_CIO_IRQ_ULPSACTIVENOT_ALL0  (1 << 30)
#define DSI_CIO_IRQ_ULPSACTIVENOT_ALL1  (1 << 31)

#define DSI_CIO_IRQ_ERROR_MASK          (DSI_CIO_IRQ_ERRSYNCESC1 | DSI_CIO_IRQ_ERRSYNCESC2 | \
                                        DSI_CIO_IRQ_ERRSYNCESC3 | DSI_CIO_IRQ_ERRESC1 | DSI_CIO_IRQ_ERRESC2 | \
                                        DSI_CIO_IRQ_ERRESC3 | DSI_CIO_IRQ_ERRCONTROL1 | \
                                        DSI_CIO_IRQ_ERRCONTROL2 | DSI_CIO_IRQ_ERRCONTROL3 | \
                                        DSI_CIO_IRQ_ERRCONTENTIONLP0_1 | DSI_CIO_IRQ_ERRCONTENTIONLP1_1 | \
                                        DSI_CIO_IRQ_ERRCONTENTIONLP0_2 | DSI_CIO_IRQ_ERRCONTENTIONLP1_2 | \
                                        DSI_CIO_IRQ_ERRCONTENTIONLP0_3 | DSI_CIO_IRQ_ERRCONTENTIONLP1_3)






#define EN_DSS			0
#define EN_DSS1			0
#define EN_DSS2			1
#define EN_TV			2

#define DISPC_CLK_SWITCH	0
#define AUTO_DSS		0
#define EN_CORE			0
#define EN_MPU			1
#define EN_IVA2			2
#define CLKTRCTRL_DSS		1

/* DSI_SYSCONFIG */
#define AUTO_IDLE		0
#define SOFT_RESET		1
#define ENWAKEUP		2
#define SIDLEMODE		4
#define CLOCKACTIVITY		9

/* DSI_CTRL */
#define DCS_CMD_ENABLE		24
#define EOT_ENABLE		19
#define TRIGGER_RESET_MODE	14
#define LINE_BUFFER		13
#define VP_DATA_BUS_WIDTH	7
#define VP_CLK_RATIO		4
#define TX_FIFO_ARBITRATION	3
#define ECC_RX_EN		2
#define CS_RX_EN		1
#define IF_EN			0

/* DSI_CLK_CTRL */
#define PLL_PWR_STATUS		29
#define PLL_PWR_CMD		31

/* DSI_VCn_CTRL */
#define DMA_RX_REQ_NB		29
#define DMA_TX_REQ_NB		23
#define VC_BUSY			15
#define MODE_SPEED		9
#define ECC_TX_EN		8
#define CS_TX_EN		7
#define BTA_EN			6
#define TX_FIFO_NOT_EMPTY	5
#define MODE			4
#define BTA_LONG_EN		3
#define BTA_SHORT_EN		2
#define SOURCE			1
#define VC_EN			0

/* DSI_VCn_IRQSTATUS */
#define PACKET_SENT_IRQ		2



#define DSI_DT_DCS_LONG_WRITE		0x39
#define DSI_DT_DCS_SHORT_WRITE_0	0x05
#define DSI_DT_DCS_SHORT_WRITE_1	0x15
#define DSI_DT_NULL_PACKET		0x09
#define DSI_DT_DCS_READ                 0x06
#define DSI_DT_RX_ACK_WITH_ERR          0x02
#define DSI_DT_RX_SHORT_READ_1          0x21
#define DSI_DT_RX_SHORT_READ_2          0x22
#define DSI_DT_RX_DCS_LONG_READ         0x1c



#define DSS_SRC_DSI1_PLL_FCLK                   0
#define DSS_SRC_DSI2_PLL_FCLK                   1
#define DSS_SRC_DSS1_ALWON_FCLK                 2

#define DCS_TEAR_OFF            		0x34
#define DCS_TEAR_ON             		0x35
#define DCS_COLUMN_ADDR         		0x2a
#define DCS_PAGE_ADDR           		0x2b


#ifndef __KERNEL_STRICT_NAMES
#ifndef __ASSEMBLY__

#define DISPLAY_SUBSYSTEM_BASE			0x48050000

volatile struct display_subsystem_registers {
	u32 revisionnumber;			//R   0x48050000   0x000
	u8  res1[0xc];
	u32 sysconfig;				//RW  0x48050010   0x010
	u32 sysstatus;				//R   0x48050014   0x014
	u32 irqstatus;				//R   0x48050018   0x018
	u8  res2[0x24];
	u32 control;				//RW  0x48050040   0x040
	u8  res3[0x18];
	u32 clk_status;				//R   0x4805005c   0x05c
};


#define DISPLAY_CONTROLLER_BASE			0x48050400

volatile struct display_controller_registers {
	u32 revision;				//R   0x48050400   0x000
	u8  res1[0xc];
	u32 sysconfig;				//RW  0x48050410   0x010
	u32 sysstatus;				//R   0x48050414   0x014
	u32 irqstatus;				//RW  0x48050418   0x018
	u32 irqenable;				//RW  0x4805041c   0x01c
	u8  res2[0x20];
	u32 control;				//RW  0x48050440   0x040
	u32 config;				//RW  0x48050444   0x044
	u8  res3[0x4];
	u32 default_color_0;			//RW  0x4805044c   0x04c
	u32 default_color_1;			//RW  0x48050450   0x050
	u32 trans_color_0;			//RW  0x48050454   0x054
	u32 trans_color_1;			//RW  0x48050458   0x058
	u32 line_status;			//R   0x4805045c   0x05c
	u32 line_number;			//RW  0x48050460   0x060
	u32 timing_h;				//RW  0x48050464   0x064
	u32 timing_v;				//RW  0x48050468   0x068
	u32 pol_freq;				//RW  0x4805046c   0x06c
	u32 divisor;				//RW  0x48050470   0x070
	u32 global_alpha;			//RW  0x48050474   0x074
	u32 size_dig;				//RW  0x48050478   0x078
	u32 size_lcd;				//RW  0x4805047c   0x07c
	u32 gfx_ba0;				//RW  0x48050480   0x080
	u32 gfx_ba1;				//RW  0x48050484   0x084
	u32 gfx_position;			//RW  0x48050488   0x088
	u32 gfx_size;				//RW  0x4805048c   0x08c
	u8  res4[0x10];
	u32 gfx_attributes;			//RW  0x480504a0   0x0a0
	u32 gfx_fifo_threshold;			//RW  0x480504a4   0x0a4
	u32 gfx_fifo_size_status;		//R   0x480504a8   0x0a8
	u32 gfx_row_inc;			//RW  0x480504ac   0x0ac
	u32 gfx_pixel_inc;			//RW  0x480504b0   0x0b0
	u32 gfx_window_skip;			//RW  0x480504b4   0x0b4
	u32 gfx_table_ba;			//RW  0x480504b8   0x0b8
	u8  res5[0x118];
	u32 data_cycle0;			//RW  0x480505d4   0x1d4
	u32 data_cycle1;			//RW  0x480505d8   0x1d8
	u32 data_cycle2;			//RW  0x480505dc   0x1dc
	u8  res6[0x40];
	u32 cpr_coef_r;				//RW  0x48050620   0x220
	u32 cpr_coef_g;				//RW  0x48050624   0x224
	u32 cpr_coef_b;				//RW  0x48050628   0x228
	u32 gfx_preload;			//RW  0x4805062c   0x22c
};


#define DSI_PROTOCOL_ENGINE_BASE		0x4804FC00

volatile struct dsi_engine_registers {
	u32 revision;				//R   0x4804FC00   0x000
	u8  res1[0xc];
	u32 sysconfig;				//RW  0x4804fc10   0x010
	u32 sysstatus;				//R   0x4804fc14   0x014
	u32 irqstatus;				//RW  0x4804fc18   0x018
	u32 irqenable;				//RW  0x4804fc1c   0x01c
	u8  res2[0x20];
	u32 ctrl;				//RW  0x4804fc40   0x040
	u8  res3[0x4];
	u32 complexio_cfg1;			//RW  0x4804fc48   0x048
	u32 complexio_irqstatus;		//RW  0x4804fc4c   0x04c
	u32 complexio_irqenable;		//RW  0x4804fc50   0x050
	u32 clk_ctrl;				//RW  0x4804fc54   0x054
	u32 timing1;				//RW  0x4804fc58   0x058
	u32 timing2;				//RW  0x4804fc5c   0x05c
	u32 vm_timing1;				//RW  0x4804fc60   0x060
	u32 vm_timing2;				//RW  0x4804fc64   0x064
	u32 vm_timing3;				//RW  0x4804fc68   0x068
	u32 clk_timing;				//RW  0x4804fc6c   0x06c
	u32 tx_fifo_vc_size;			//RW  0x4804fc70   0x070
	u32 rx_fifo_vc_size;			//RW  0x4804fc74   0x074
	u32 complexio_cfg2;			//RW  0x4804fc78   0x078
	u32 rx_fifo_vc_fullness;		//R   0x4804fc7c   0x07c
	u32 vm_timing4;				//RW  0x4804fc80   0x080
	u32 tx_fifo_vc_emptiness;		//R   0x4804fc84   0x084
	u32 vm_timing5;				//RW  0x4804fc88   0x088
	u32 vm_timing6;				//RW  0x4804fc8c   0x08c
	u32 vm_timing7;				//RW  0x4804fc90   0x090
	u32 stopclk_timing;			//RW  0x4804fc94   0x094
	u8  res4[0x68];
	u32 vc0_ctrl;				//RW  0x4804fd00   0x100
	u32 vc0_te;				//RW  0x4804fd04   0x104
	u32 vc0_long_packet_header;		//W   0x4804fd08   0x108
	u32 vc0_long_packet_payload;		//W   0x4804fd0c   0x10c
	u32 vc0_short_packet_header;		//RW  0x4804fd10   0x110
	u8  res5[0x4];
	u32 vc0_irqstatus;			//RW  0x4804fd18   0x118
	u32 vc0_irqenable;			//RW  0x4804fd1c   0x11c
	u32 vc1_ctrl;				//RW  0x4804fd20   0x120
	u32 vc1_te;				//RW  0x4804fd24   0x124
	u32 vc1_long_packet_header;		//W   0x4804fd28   0x128
	u32 vc1_long_packet_payload;		//W   0x4804fd2c   0x12c
	u32 vc1_short_packet_header;		//RW  0x4804fd30   0x130
	u8  res6[0x4];
	u32 vc1_irqstatus;			//RW  0x4804fd38   0x138
	u32 vc1_irqenable;			//RW  0x4804fd3c   0x13c
	u32 vc2_ctrl;				//RW  0x4804fd40   0x140
	u32 vc2_te;				//RW  0x4804fd44   0x144
	u32 vc2_long_packet_header;		//W   0x4804fd48   0x148
	u32 vc2_long_packet_payload;		//W   0x4804fd4c   0x14c
	u32 vc2_short_packet_header;		//RW  0x4804fd50   0x150
	u8  res7[0x4];
	u32 vc2_irqstatus;			//RW  0x4804fd58   0x158
	u32 vc2_irqenable;			//RW  0x4804fd5c   0x15c
	u32 vc3_ctrl;				//RW  0x4804fd60   0x160
	u32 vc3_te;				//RW  0x4804fd64   0x164
	u32 vc3_long_packet_header;		//W   0x4804fd68   0x168
	u32 vc3_long_packet_payload;		//W   0x4804fd6c   0x16c
	u32 vc3_short_packet_header;		//RW  0x4804fd70   0x170
	u8  res8[0x4];
	u32 vc3_irqstatus;			//RW  0x4804fd78   0x178
	u32 vc3_irqenable;			//RW  0x4804fd7c   0x17c
};


#define DSI_PHY_BASE				0x4804FE00

volatile struct dsi_phy_registers {
	u32 register0;				//RW  0x4804fe00   0x000
	u32 register1;				//RW  0x4804fe04   0x004
	u32 register2;				//RW  0x4804fe08   0x008
	u32 register3;				//RW  0x4804fe0c   0x00c
	u32 register4;				//RW  0x4804fe10   0x010
	u32 register5;				//R   0x4804fe14   0x014
};


#define DSI_PLL_BASE				0x4804FF00

volatile struct dsi_pll_registers {
	u32 control;				//RW  0x4804ff00   0x000
	u32 status;				//R   0x4804ff04   0x004
	u32 go;					//RW  0x4804ff08   0x008
	u32 configuration1;			//RW  0x4804ff0c   0x00c
	u32 configuration2;			//RW  0x4804ff10   0x010
};


#define DSS_PRM_BASE				0x48306E58

volatile struct dss_prm_registers {
	u32 rstst;				//RW  0x48306e58   0x000
	u8  res1[0x44];
	u32 wken;				//RW  0x48306ea0   0x00
	u8  res2[0x24];
	u32 wkdep;				//RW  0x48306ec8   0x00
	u8  res3[0x14];
	u32 pwstctrl;				//RW  0x48306ee0   0x00
	u32 pwstst;				//R   0x48306ee4   0x00
	u32 prepwstst;				//RW  0x48306ee8   0x00
};


#define DSS_CM_BASE				0x48004E00

volatile struct dss_cm_registers {
	u32 fclken;				//RW  0x48004e00   0x00
	u8  res1[0xc];
	u32 iclken;				//RW  0x48004e10
	u8  res2[0xc];
	u32 idlest;				//R   0x48004e20
	u8  res3[0xc];
	u32 autoidle;				//RW  0x48004e30
	u8  res4[0xc];
	u32 clksel;				//RW  0x48004e40
	u32 sleepdep;				//RW  0x48004e44
	u32 clkstctrl;				//RW  0x48004e48
	u32 clkstst;				//R   0x48004e4c
};



/*

struct orion_display_system {
	struct display_subsystem_registers*  dss;
        struct display_controller_registers* dispc;
        struct dsi_engine_registers*         dsi;
        struct dsi_phy_registers*            phy;
        struct dsi_pll_registers*            pll;
        struct dss_cm_registers*             dcm;
        struct dss_prm_registers*            dprm;
} display;


struct channel_num {
	u32* vc_ctrl;
	u32* vc_sysconfig;
	
	
};

struct device {
	struct channel_num* channel;
	int vc_num;
	dsi_vc_port port;
};



display = {
	.dss = (struct display_subsystem_registers*)DISPLAY_SUBSYSTEM_BASE;
	
	
	
};

*/






//DISPC

typedef enum {
	color_depth_12_bit,
	color_depth_16_bit,
	color_depth_18_bit,
	color_depth_24_bit
} tftdatalines;				//Number of lines of the LCD interface

typedef enum {
	LCD_DISPLAY_STN,
	LCD_DISPLAY_TFT,
} lcd_display_type;

typedef enum {
        PARALLELMODE_BYPASS,           /* MIPI DPI */
        PARALLELMODE_RFBI,             /* MIPI DBI */
        PARALLELMODE_DSI,
} parallel_interface_mode;

struct orion_video_timings {
	/* Unit: pixels */
	u16 x_res;
	/* Unit: pixels */
	u16 y_res;
	/* Unit: KHz */
	u32 pixel_clock;
	/* Unit: pixel clocks */
	u16 hsw;	/* Horizontal synchronization pulse width */
	/* Unit: pixel clocks */
	u16 hfp;	/* Horizontal front porch */
	/* Unit: pixel clocks */
	u16 hbp;	/* Horizontal back porch */
	/* Unit: line clocks */
	u16 vsw;	/* Vertical synchronization pulse width */
	/* Unit: line clocks */
	u16 vfp;	/* Vertical front porch */
	/* Unit: line clocks */
	u16 vbp;	/* Vertical back porch */
};




struct orion_display {
	struct display_controller_registers* dispc;
	tftdatalines            color_depth;
	lcd_display_type        display_type;
	parallel_interface_mode interface_mode;
};


typedef enum {
        l4_interconnect,
        video_port
} dsi_vc_port;

typedef enum {
        command_mode,
        video_mode
} dsi_vc_mode;

typedef enum {
        manual_bta,
        automatic_bta
} dsi_bta_mode;

typedef enum {
        dma_req0,
        dma_req1,
        dma_req2,
        dma_req3,
        no_dma
} dsi_dma_req;


enum dss_clock {
        DSS_CLK_ICK     = 1 << 0,
        DSS_CLK_FCK1    = 1 << 1,
        DSS_CLK_FCK2    = 1 << 2,
        DSS_CLK_54M     = 1 << 3,
        DSS_CLK_96M     = 1 << 4,
};


//=========================================================================================








#endif /*__ASSEMBLY__ */
#endif /* __KERNEL_STRICT_NAMES */

#endif






























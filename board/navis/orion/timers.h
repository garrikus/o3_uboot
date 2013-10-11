//#include <asm/io.h>
//#include <asm/arch-omap3/sys_proto.h>
//#include <twl4030.h>

#ifndef __TIMERS_H__
#define __TIMERS_H__

#include <asm/io.h>
#include <asm/arch-omap3/sys_proto.h>
#include <twl4030.h>


//------  TCLR  -------
#define ST			0
#define AR			1
#define PTV			4
#define PRE			5
#define CE			6
#define SCPWM			7
#define TCM			9
#define TRG			11
#define PT			12
#define CAPT_MODE		13
#define GPO_CFG			14

#define EN_GPT2			3
#define EN_GPT3			4
#define EN_GPT4			5
#define EN_GPT5			6
#define EN_GPT6			7
#define EN_GPT7			8
#define EN_GPT8			9
#define EN_GPT9			10

#define CLKSEL_GPT3		1
#define CLKSEL_GPT4		2
#define CLKSEL_GPT8		6
#define CLKSEL_GPT9		7

#define SYS_CLKIN_SEL		2


typedef enum {
	OSC_SYS_CLK_12,
	OSC_SYS_CLK_13,
	OSC_SYS_CLK_19_2,
	OSC_SYS_CLK_26,
	OSC_SYS_CLK_38_4,
	OSC_SYS_CLK_16_8
} sys_clk;

typedef enum {
	gpt1 = 1,
	gpt2,
	gpt3,
	gpt4,
	gpt5,
	gpt6,
	gpt7,
	gpt8,
	gpt9,
	gpt10,
	gpt11
} tmr_num;

typedef enum {
	_32k_fclk,
	system_clk
} source_clk;

typedef enum {
	one_shot,
	autoreload
} tmr_reload;

typedef enum {
	count,
	ctc,
	fast_pwm,
	phase_pwm
} tmr_mode;

typedef enum {
	div2,
	div4,
	div8,
	div16,
	div32,
	div64,
	div128,
	div256,
	no_prescaler
} tmr_divisor;

typedef struct {
	struct gptimer* tmr;
	struct prcm*    prcm_base;
	struct prm*     prm_base;
	tmr_mode        mode;
	tmr_reload      reload;
	source_clk      source;
	tmr_divisor     divisor;
	tmr_num         num;
} timer_t;

void inline select_sys_clk(struct prm*, sys_clk);
int config_timer(timer_t*,
		 tmr_mode,
		 tmr_reload,
		 source_clk,
		 tmr_divisor,
		 u32,
		 u32,
		 u32);
timer_t* init_timer(tmr_num);
void inline start_timer(timer_t*);
void inline stop_timer(timer_t*);

#endif

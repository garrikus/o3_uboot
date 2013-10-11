
#include "timers.h"

static timer_t gp_timer[11];

//--------------------------------------------------------------------------------------------

void inline select_sys_clk(struct prm* p, sys_clk clk)
{
    sr32(&p->clksel, SYS_CLKIN_SEL, 3, clk);		//set system clk
}

//--------------------------------------------------------------------------------------------

int config_timer(timer_t*    t,
		 tmr_mode    md,
		 tmr_reload  rld,
		 source_clk  clk,
		 tmr_divisor div,
		 u32 	     restart_value,
		 u32 	     match_value,
		 u32 	     counter_value)
{
    int error = -1;

    if(t == NULL || t->tmr == NULL)
				return error;

    t->mode      = md;
    t->reload    = rld;
    t->source    = clk;
    t->divisor   = div;

    u32 i = 0,
	j = 0,
	val = readl(&t->prm_base->clksrc_ctrl);

    if (!(val & SYSCLKDIV_2))
		writel((val & (~(3 << 6))) | (1 << 6), &t->prm_base->clksrc_ctrl);

    switch(t->num)
    {
	case gpt1:
		break;
	case gpt2:
		break;
	case gpt3:
		i = EN_GPT3;
		j = CLKSEL_GPT3;
		break;
	case gpt4:
		i = EN_GPT4;
		j = CLKSEL_GPT4;
		break;
	case gpt8:
		i = EN_GPT8;
		j = CLKSEL_GPT8;
		break;
	case gpt9:
		i = EN_GPT9;
		j = CLKSEL_GPT9;
		break;
	default:
		error = -1;
		return error;
    }

    val = readl(&t->prcm_base->fclken_per) | (1 << i);	//function clock FCLK for GPtimer#n is enabled
    writel(val, &t->prcm_base->fclken_per);

//    sr32(&t.prm_base->clksel, SYS_CLKIN_SEL, 3, OSC_SYS_CLK_26);		//set system clk at 26 MHz
    sr32(&t->prcm_base->clksel_per, j, 1, t->source);	//source for GPTn is clk

    writel(restart_value, &t->tmr->tldr);		//start counting after reset at restart_value
    writel(match_value, &t->tmr->tmar);			//set match register to 
    writel(counter_value, &t->tmr->tcrr);		//set GPTn to counter_value

    val = 0;

    if(t->mode == fast_pwm) val = (1 << PT) | (1 << TRG) | (1 << CE) | (1 << AR);
    if(t->divisor < 8) val |= ((1 << PRE) | (t->divisor << PTV));

    writel(val, &t->tmr->tclr);

    error = 0;

    return error;
}

//--------------------------------------------------------------------------------------------

timer_t* init_timer(tmr_num n)
{
    if(n > 11 || n < 0)
    {
        puts("This timer can't init!\n");
        return NULL;
    }

    timer_t* t = &gp_timer[n-1];

    memset(t, 0, sizeof(t));

    t->prcm_base = (struct prcm*)PRCM_BASE;
    t->prm_base  = (struct prm*)PRM_BASE;
    t->num       = n;

    switch(n)
    {
	case gpt1:
		t->tmr = (struct gptimer*)OMAP34XX_GPT1;
		break;
	case gpt2:
		t->tmr = (struct gptimer*)OMAP34XX_GPT2;
		break;
	case gpt3:
		t->tmr = (struct gptimer*)OMAP34XX_GPT3;
		break;
	case gpt4:
		t->tmr = (struct gptimer*)OMAP34XX_GPT4;
		break;
	case gpt5:
		t->tmr = (struct gptimer*)OMAP34XX_GPT5;
		break;
	case gpt6:
		t->tmr = (struct gptimer*)OMAP34XX_GPT6;
		break;
	case gpt7:
		t->tmr = (struct gptimer*)OMAP34XX_GPT7;
		break;
	case gpt8:
		t->tmr = (struct gptimer*)OMAP34XX_GPT8;
		break;
	case gpt9:
		t->tmr = (struct gptimer*)OMAP34XX_GPT9;
		break;
	case gpt10:
		t->tmr = (struct gptimer*)OMAP34XX_GPT10;
		break;
	case gpt11:
		t->tmr = (struct gptimer*)OMAP34XX_GPT11;
		break;
	default:
		t->tmr = NULL;
		return  NULL;
    }

    config_timer(t, count, autoreload, system_clk, no_prescaler, 0, 0, 0);
    select_sys_clk(t->prm_base, OSC_SYS_CLK_26);
//    sr32(&t->prm_base->clksel, SYS_CLKIN_SEL, 3, OSC_SYS_CLK_26);

    return t;
}

//--------------------------------------------------------------------------------------------

void inline start_timer(timer_t* t)
{
    sr32(&t->tmr->tclr, ST, 1, 1);				//enable clock - START timer
}

//--------------------------------------------------------------------------------------------

void inline stop_timer(timer_t* t)
{
    sr32(&t->tmr->tclr, ST, 1, 0);				//STOP timer
}

//--------------------------------------------------------------------------------------------
/*
timer_t* set_pwm_timer(tmr_num n, u32 freq)
{
    if(n > 11 || n < 0)
    {
        puts("This timer can't init!\n");
        return NULL;
    }

    timer_t* t = &gp_timer[n-1];

    memset(t, 0, sizeof(t));

    t->prcm_base = (struct prcm*)PRCM_BASE;
    t->prm_base  = (struct prm*)PRM_BASE;
    t->num       = n;

    switch(n)
    {
	case gpt1:
		t->tmr = (struct gptimer*)OMAP34XX_GPT1;
		break;
	case gpt2:
		t->tmr = (struct gptimer*)OMAP34XX_GPT2;
		break;
	case gpt3:
		t->tmr = (struct gptimer*)OMAP34XX_GPT3;
		break;
	case gpt4:
		t->tmr = (struct gptimer*)OMAP34XX_GPT4;
		break;
	case gpt5:
		t->tmr = (struct gptimer*)OMAP34XX_GPT5;
		break;
	case gpt6:
		t->tmr = (struct gptimer*)OMAP34XX_GPT6;
		break;
	case gpt7:
		t->tmr = (struct gptimer*)OMAP34XX_GPT7;
		break;
	case gpt8:
		t->tmr = (struct gptimer*)OMAP34XX_GPT8;
		break;
	case gpt9:
		t->tmr = (struct gptimer*)OMAP34XX_GPT9;
		break;
	case gpt10:
		t->tmr = (struct gptimer*)OMAP34XX_GPT10;
		break;
	case gpt11:
		t->tmr = (struct gptimer*)OMAP34XX_GPT11;
		break;
	default:
		t->tmr = NULL;
		return  NULL;
    }

    config_timer(t, count, autoreload, system_clk, no_prescaler, 0, 0, 0);
    select_sys_clk(t->prm_base, OSC_SYS_CLK_26);

    return t;
}
*/

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
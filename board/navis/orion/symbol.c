#include "font.h"
#include <asm/io.h>
//#include <asm/arch-omap3/sys_proto.h>


static void draw_string(int x, int line, unsigned char* points, int len)
{
    int offset,
        y,
        dot,
        i, j;
    unsigned int addr, color;

    for(j = 0; j < len; j++) {
        for(i = 7; i >= 0; i--) {
            if(points[j] & (1 << i))
                color = 0xffffffff;
            else
                color = 0;

                y = 480 * (line - 1) + 1;
                dot = y + x, x++;
                offset = 4 * (dot - 1);
                addr = 0x8fc00000 + offset;
                *((ulong*)addr) = (ulong)color;
        }
    }
}

void sym_to_frame(int x, int line, symbol* chr)
{
    int i;
    unsigned char *p;

    for(i = 0; i < chr->row; i++) {
        p = chr->sym + i * chr->column;
        draw_string(x, line++, p, chr->column);
        draw_string(x, line++, p, chr->column);
    }
}

void string_to_frame(int x, int y, symbol* s, int len)
{
    int i;

    for(i = 0; i < len; x += s[++i].distance)
					sym_to_frame(x, y, &s[i]);
}

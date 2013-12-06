#include <asm/io.h>
#include <asm/arch-omap3/sys_proto.h>


inline u32 setvalue(u32 source, u8 offset, u8 count, u32 data)
{
	if (count > (offset + 1)) return 0;

	++offset;
	offset -= count;

	u32 mask = (1 << count),
	    val1 = ~data;

	mask--;
	val1 &= mask;
	val1 <<= offset;
	val1 = ~val1;

	return ((source & val1) | (data << offset));
}

inline void setv32(u32 *source, u8 offset, u8 count, u32 data)
{
	if (count > (offset + 1)) return;

	++offset;
	offset -= count;

	u32 mask = (1 << count),
	    val1 = ~data;

	mask--;
	val1 &= mask;
	val1 <<= offset;
	val1 = ~val1;

	*source = ((*source & val1) | (data << offset));
}

inline void r32setv(void *rg, u8 offset, u8 count, u32 data)
{
	writel(setvalue(readl(rg), offset, count, data), rg);
}

inline void showbits(const char *s, u32 value)
{
	int i;

	if (s) {
		puts(s);
		putc(' ');
	}

	puts("bits:");

	for (i = 32; i; i--) {
		if (!(i % 4)) putc(' ');

		printf("%d", (value >> (i - 1)) & 0x1);
	}

	putc('\n');
}

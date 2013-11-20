#include <config.h>
#include <common.h>
#include <command.h>
#include <part.h>
#include <fat.h>
#include <mmc.h>
#include <nand.h>


#define SRAM_ADDR4IMAGE 0x80000000UL

/* XXX: This is wrong in a way. Should be retrieved from MTD subsystem. */
#define NANDPAGE_SIZE   0x800

/* XXX: This is also wrong. The total NAND size should be retrieved from MTD. */
static struct nand_part_map {
    unsigned int offset;
    unsigned int size;
} nand_parts[] = {
    {0x00000000, 0x00050000}, /* X-Loader MLO */
    {0x00080000, 0x001c0000}, /* U-boot       */
    {0x00280000, 0x00500000}, /* Linux kernel */
    {0x00780000, 0x3e4e0000}, /* rootfs image */
    {0x3ec60000, 0x00020000}, /* imgs magic number */
    {0x3ec80000, 0x00180000}, /* img1 */
    {0x3ee00000, 0x00180000}, /* img2 */
    {0x3ef80000, 0x00180000}, /* img3 */
    {0x3f100000, 0x00180000}  /* img4 */
};

int do_sd2n(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    int     force_flag = 0;
    char *  bin_names[] = {"o_MLO", "o_u-boot.bin", "o_uImage", "o_rootfs", "magic",
    			   "img1.raw", "img2.raw", "img3.raw", "img4.raw"};
    int     what_id;
    char *  fname_ptr;
    long    addr,count,filesize,wrsize; 
    size_t  length;
    static int repeat = 5;

    block_dev_desc_t *dev_desc=NULL;
    nand_info_t *nand;
    nand_erase_options_t opts;

    if (argc < 2) {
	printf ("usage: sd2n <what> [force]\n");
	return 1;
    }
	
    if (strcmp (argv[1], "mlo") == 0){
	fname_ptr = bin_names[0];
	what_id   = 0;
    }else
	if (strcmp (argv[1], "uboot") == 0){
	    fname_ptr = bin_names[1];
	    what_id   = 1;
	}else
	    if (strcmp (argv[1], "uImage") == 0){
		fname_ptr = bin_names[2];
		what_id   = 2;
	    }else
		if (strcmp (argv[1], "rootfs") == 0){
		    fname_ptr = bin_names[3];
		    what_id   = 3;
		}else{
		    fname_ptr = 0x00;
		    what_id   = -1;
		}
    
    if(strcmp (argv[1], "img") == 0) {
		fname_ptr = bin_names[repeat];
                what_id   = repeat;
    }

    if (fname_ptr == 0x00){
	printf ("what? do you want (mlo,uboot,uImage,rootfs,img)\n");
	return 1;
    }

    if (argc > 2 && strcmp (argv[2], "force") == 0)
    	force_flag = 1;

    printf ("Ready to copy from [%s] with force = %d\n", fname_ptr, force_flag);


    /* Ensure mmc is available */
    if (mmc_legacy_init(1) != 0) {
	puts("No MMC card found\n");
	return 1;
    }
    else
	puts("MMC found\n");

    if ( force_flag )
    {
	/* Check device availability */
	if (nand_curr_device < 0 || nand_curr_device >= CONFIG_SYS_MAX_NAND_DEVICE ||
	    !nand_info[nand_curr_device].name) {
		puts("\nno devices available\n");
		return 1;
	}
	nand = &nand_info[nand_curr_device];

	/* Erase all the NAND required */
	opts.length = nand_parts[what_id].size;
	opts.offset = nand_parts[what_id].offset;
	opts.quiet  = 0;
	opts.jffs2  = 0;
	opts.scrub  = 0;

	printf( "Will now erase nand: offs 0x%x, size 0x%x\n", opts.offset, opts.length );
	/* XXX: Take care with that "effective" condition */
	if (nand_erase_opts(nand, &opts))
	{
	    printf("Unable to erase NAND\n");
	    return 1;
	};
    }

    /* Check NAND is Erase */

    /* Load that file and Find out its filesize */
    dev_desc=get_dev("mmc", 0);
    if (dev_desc==NULL) {
	puts ("\n** Invalid boot device **\n");
	return 1;
    }
    if (fat_register_device(dev_desc,1)!=0) {
	printf ("\n** Unable to use for fatload **\n");
	return 1;
    }
    
    filesize = file_fat_read (fname_ptr, (unsigned char *) SRAM_ADDR4IMAGE, 0);
    if(filesize==-1) {
	printf("\n** Unable to read \"%s\"  **\n",fname_ptr);
	return 1;
    }

    /* Calculate NANDPAGE_SIZE-padded space size */
    count = NANDPAGE_SIZE - (filesize%NANDPAGE_SIZE);
    if (count == NANDPAGE_SIZE)
	count = 0;

    wrsize = filesize+count;
    
    /* Pad-Fill with 0xFF */
    addr = SRAM_ADDR4IMAGE+filesize;
    while (count > 0) {
	*((u_char *)addr) = (u_char)0xff;
	addr++;
	count--;
    }

    printf( "Fload to 0x%x fsize 0x%x, wrsize 0x%x\n", SRAM_ADDR4IMAGE, filesize, wrsize );

    /* Put what into NAND */
    if (force_flag)
    {
	/* Don't do flash programming unless forced */
	length = (size_t)wrsize;

	printf( "Will now write nand: offs 0x%x, size 0x%x\n", nand_parts[what_id].offset, length );
	if (nand_write_skip_bad(nand, (loff_t)nand_parts[what_id].offset,
		    &length, (u_char *)(SRAM_ADDR4IMAGE)))
	{
	    printf("Unable to program NAND\n");
	    return 1;
	}

    /*
     * FIXME: Probably the right way to do it is using nand_parts[].
     * We keep NAND related info in one place that way rather than
     * doing by hand, which is error prone.
     */
	if(repeat == 8) {
		repeat = 5;
		
		DECLARE_GLOBAL_DATA_PTR;
		char data[10], buff[10], addr[10];
		unsigned size = 0x20000;
		
		sprintf(data, "0x%x", gd->mnumber);
		sprintf(buff, "0x%x", (unsigned int)SRAM_ADDR4IMAGE);

		char* s[] = {"do_mem_mw", buff, data, "", ""};

		do_mem_mw(NULL, 0, 3, s);
		s[0] = "do_nand";
		s[1] = "erase";
		sprintf(addr, "0x%x", (nand_parts[4].offset));
		s[2] = addr;		//"0x3ec60000";
		sprintf(data, "0x%x", size);
		s[3] = data;		//"0x20000";
		do_nand(NULL, 0, 4, s);
		s[1] = "write";
		s[2] = buff;		//"0x80000000";
		s[3] = addr;		//"0x3ec60000";
		sprintf(data, "0x%x", size);
		s[4] = data;            //"0x20000";
		do_nand(NULL, 0, 5, s);
	} else
		if(strcmp (argv[1], "img") == 0) {
			char* s[] = {"do_sd2n", "img", "force"};
			repeat++;
			do_sd2n(NULL, 0, 3, s);
	}
    }

    /* Done! */
    return 0;
}

U_BOOT_CMD(
	sd2n,	3,	0,	do_sd2n,
	"copy various system binaries from SD card to NAND",
	"<binary> [force]\n"
	"    - copy <binary> = mlo|uboot|uImage|rootfs to a predefind NAND location\n"
	"      doesn't touch NAND unless forced"
);

inline unsigned get_addr(unsigned bl)
{   
    if(bl > 8)
		return 0x3ec60000;
			            
    return nand_parts[bl].offset;
}



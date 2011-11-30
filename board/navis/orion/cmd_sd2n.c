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
    {0x00780000, 0x3f880000}  /* rootfs image */
};

int do_sd2n(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    int     force_flag = 0;
    char *  bin_names[] = { "o_MLO", "o_u-boot.bin", "o_uImage", "o_rootfs" };
    int     what_id;
    char *  fname_ptr;
    long    addr,count,filesize,wrsize; 
    size_t  length;
    int     ecc_type;

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

    if (fname_ptr == 0x00){
	printf ("what? do you want (mlo,uboot,uImage,rootfs)\n");
	return 1;
    }

    if (what_id < 2)
	ecc_type = 2;
    else
	ecc_type = 1;

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
	omap_nand_switch_ecc(NAND_ECC_HW, ecc_type);

	printf( "Will now write nand: offs 0x%x, size 0x%x\n", nand_parts[what_id].offset, length );
	if (nand_write_skip_bad(nand, (loff_t)nand_parts[what_id].offset,
		    &length, (u_char *)(SRAM_ADDR4IMAGE)))
	{
	    printf("Unable to program NAND\n");
	    return 1;
	};
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

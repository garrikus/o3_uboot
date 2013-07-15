#!/bin/bash
#
#

export BUILD_DIR=$PWD/build/

make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- distclean
make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- omap3_orion_config
make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi-

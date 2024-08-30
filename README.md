# FUSE-Dumper
Nintendo Swtich homebrew dumping FUSE to sdcard.

FUSE dumping is achieved by putting "usb" sysmodule into debug state and finding FUSE IO region mapped by this sysmodule.

It may not work if something else is currently debugging processes.

Just run homebrew and FUSE will be dumped to the root of sdcard as "fuse_dump.bin".

For offsets reference look at Hekate source code: https://github.com/CTCaer/hekate/blob/master/bdk/soc/fuse.h

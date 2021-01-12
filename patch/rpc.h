
#ifndef _RPC_H_
#define _RPC_H_

#include <kernel.h>
#include <sifrpc.h>
#include <sbv_patches.h>
#include <syscallnr.h>
#include <libcdvd.h>
#include <debug.h>

#include "loadfile.h"


#define USBSERV_BUFSIZE 	(1024 * 8)

/* IRX Modules and elf loader */

extern void *memdisk_irx;
extern int  size_memdisk_irx;
extern void *eesync_irx;
extern int  size_eesync_irx;
extern void *usbd_irx;
extern int  size_usbd_irx;
extern void *usb_mass_irx;
extern int  size_usb_mass_irx;
extern void *usbserv_irx;
extern int  size_usbserv_irx;


// rpc.c
int rpcUSBInit(void);
int rpcUSBReset(void);
int rpcUSBopen(char *filename, int flags);
int rpcUSBwrite(int fd, u8 *buf, int size);
int rpcUSBclose(int fd);
int rpcUSBread(int fd, void *buf, int size);
int rpcUSBseek(int fd, int offset, int whence);
int rpcUSBSync(int mode, int *cmd, int *result);
int rpcUSBSyncNB(int mode, int *cmd, int *result);

#endif // _RPC_H_

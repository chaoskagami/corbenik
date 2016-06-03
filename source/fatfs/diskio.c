/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include <stddef.h>
#include "diskio.h"		/* FatFs lower layer API */
#include "sdmmc.h"

#define TYPE_NONE       0
#define TYPE_SDCARD     (1<<4)

#define SUBTYPE_NONE    5

typedef struct {
    BYTE  type;
    BYTE  subtype;
} FATpartition;

typedef struct {
    DWORD offset;
    DWORD size;
    BYTE  keyslot;
} SubtypeDesc;

FATpartition DriveInfo[11] = {
    { TYPE_SDCARD,  SUBTYPE_NONE },     // 0 - SDCARD
};

/*-----------------------------------------------------------------------*/
/* Get actual FAT partition type helper                                  */
/*-----------------------------------------------------------------------*/

static inline BYTE get_partition_type(
    __attribute__((unused))
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
    return DriveInfo[pdrv].type;
}



/*-----------------------------------------------------------------------*/
/* Get Drive Subtype helper                                              */
/*-----------------------------------------------------------------------*/

static inline SubtypeDesc* get_subtype_desc(
    __attribute__((unused))
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
    return NULL;
}



/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	__attribute__((unused))
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	__attribute__((unused))
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
    if (pdrv == 0) { // a mounted SD card is the preriquisite for everything else
        if (!sdmmc_sdcard_init()) return STA_NODISK;
    }
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	__attribute__((unused))
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
    BYTE type = get_partition_type(pdrv);

    if (type == TYPE_NONE) {
        return RES_PARERR;
    } else if (type == TYPE_SDCARD) {
        if (sdmmc_sdcard_readsectors(sector, count, buff))
            return RES_PARERR;
    }

	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	__attribute__((unused))
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
    BYTE type = get_partition_type(pdrv);

    if (type == TYPE_NONE) {
        return RES_PARERR;
    } else if (type == TYPE_SDCARD) {
        if (sdmmc_sdcard_writesectors(sector, count, (BYTE *)buff))
            return RES_PARERR;
    }

	return RES_OK;
}
#endif



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	__attribute__((unused))
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	__attribute__((unused))
	BYTE cmd,		/* Control code */
	__attribute__((unused))
	void *buff		/* Buffer to send/receive control data */
)
{
    BYTE type = get_partition_type(pdrv);

    switch (cmd) {
        case GET_SECTOR_SIZE:
            *((DWORD*) buff) = 0x200;
            return RES_OK;
        case GET_SECTOR_COUNT:
            if (type == TYPE_SDCARD) { // SD card
                *((DWORD*) buff) = getMMCDevice(1)->total_size;
            }
            return RES_OK;
        case GET_BLOCK_SIZE:
            *((DWORD*) buff) = 0x2000;
            return RES_OK;
        case CTRL_SYNC:
            // nothing else to do here - sdmmc.c handles the rest
            return RES_OK;
    }

	return RES_PARERR;
}
#endif

/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for Petit FatFs (C)ChaN, 2014      */
/*-----------------------------------------------------------------------*/

#include "diskio.h"
#include "string.h"
#include "../sdcard/bsp_spi_sdcard.h"
#include "../uart.h"

#define SD_BLOCKSIZE     512//SDCardInfo.CardBlockSize 

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (void)
{
	DSTATUS stat = STA_NOINIT;

	if(SD_Init()==SD_RESPONSE_NO_ERROR)				
		stat &= ~STA_NOINIT;
	else 
		stat = STA_NOINIT;

	// Put your code here

	return stat;
}



/*-----------------------------------------------------------------------*/
/* Read Partial Sector                                                   */
/*-----------------------------------------------------------------------*/
uint8_t pbuff[SD_BLOCKSIZE];

DRESULT disk_readp (
	BYTE* buff,		/* Pointer to the destination object */
	DWORD sector,	/* Sector number (LBA) */
	UINT offset,	/* Offset in the sector */
	UINT count		/* Byte count (bit15:destination) */
)
{
	DRESULT res;
	SD_Error SD_state = SD_RESPONSE_NO_ERROR;

	// SD_state=SD_ReadMultiBlocks(buff,(uint64_t)sector*SD_BLOCKSIZE,SD_BLOCKSIZE,count);
	SD_state=SD_ReadBlock(pbuff,(uint64_t)sector*SD_BLOCKSIZE,SD_BLOCKSIZE);
	memcpy(buff,pbuff+offset,count);

	// print("sector:");
	// printhex(sector);
	// print(",offset:");
	// printhex(offset);
	// print(",offset:");
	// printhex(count);
	// println("");

	if(SD_state!=SD_RESPONSE_NO_ERROR)
		res = RES_PARERR;
	else
		res = RES_OK;	

	return res;
}



/*-----------------------------------------------------------------------*/
/* Write Partial Sector                                                  */
/*-----------------------------------------------------------------------*/

DRESULT disk_writep (
	const BYTE* buff,		/* Pointer to the data to be written, NULL:Initiate/Finalize write operation */
	DWORD sc		/* Sector number (LBA) or Number of bytes to send */
)
{
	DRESULT res = RES_PARERR;


	if (!buff) {
		if (sc) {

			// Initiate write process

		} else {

			// Finalize write process

		}
	} else {

		// Send data to the disk

	}

	return res;
}


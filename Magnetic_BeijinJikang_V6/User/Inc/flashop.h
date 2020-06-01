/*************************************************************************************************
                                            flashop.h
                            This file is the FLASH operator header file
                        Edited by Xieyuyin 2019-09-28 Soway technology Co., Ltd
*************************************************************************************************/
#ifndef __FLASHOP_H
#define __FLASHOP_H
//Include files
#include "Baseincludes.h"
/*************************************************************************************************
Chip flash started: 0x08000000
Boot occupy falsh:  12K Bytes
Applicatioon falsh: 0x08003000
Back Company infor: 0x08003000 ~ 0x080031FF (512 Bytes)
Back parameter inf: 0x08003200 ~ 0x080037FF

Company infor user: 0x08003800 ~ 0x080039FF (512 Bytes)
Parameter inf user: 0x08003A00 ~ 0x08003FFF
Application start:  0x08004000
*************************************************************************************************/

#define FLASH_PAGE_EXPONENT         11

#ifdef IAP_REMAP_INT
    #define SOWAYFLASH_BKUPPARA_ADDR    0x08003000  //Backup parameter 2K bytes
    #define SOWAYFLASH_OP_BASE_ADDR     0x08003800  //System parameter 2K bytes
    #define SOWAYFLASH_OP_PARA_ADDR     0x08003A00  //0X08003800 ~ 0X080039FF save company string infor
    #define SOWAYAPP_START_ADDR         0x08004000  //Soway application start address
#else
    #define SOWAYFLASH_BKUPPARA_ADDR    0x0800F000  //Backup parameter 2K bytes
    #define SOWAYFLASH_OP_BASE_ADDR     0x0800F800  //System parameter 2K bytes
    #define SOWAYFLASH_OP_PARA_ADDR     0x0800FA00  //0X08003800 ~ 0X080039FF save company string infor
#endif

#define SOWAYFLASH_PAGE_SIZE        2048        /* FLASH Page Size 2K  */
#define CHIPFLASH_MAXIMUM_ADDR      0x0800FFFF  //According chip type modify
#define CHIPFLASH_EDGE_ADDR         (CHIPFLASH_MAXIMUM_ADDR + 1)
#define SOWAYAPP_FLASH_PAGES        ((CHIPFLASH_EDGE_ADDR - SOWAYAPP_START_ADDR) >> FLASH_PAGE_EXPONENT)


//Functions declare
bool FLASHOP_EraseSpecifiedFlashPages( u32 startaddr, u16 npages );
bool FLASHOP_WriteData2SpecifiedFlash( u32 startaddr, u32 *psrc, u16 wlen );
bool FLASHOP_ReadDataFromChipFlash( u8* pdest, u32 op_addr, u32 dwbufsize, u32 dwlen );

#endif

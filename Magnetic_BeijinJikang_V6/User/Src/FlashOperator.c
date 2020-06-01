/*************************************************************************************************
                                            FLASHOP.C
                            This file is the FLASH operator C source file
                        Edited by Xieyuyin 2019-09-28 Soway technology Co., Ltd
*************************************************************************************************/
//Include files
#include "flashop.h"

//Functions implemnet
/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
bool FLASHOP_EraseSpecifiedFlashPages( u32 startaddr, u16 npages )
{
    if (!npages)
        return false;

    if (startaddr + npages * SOWAYFLASH_PAGE_SIZE > CHIPFLASH_EDGE_ADDR)
        return false;
    
    FLASH_Unlock();
    __disable_irq();
    while (npages --)
    {
#ifdef USE_IDWDG
            FEED_I_WATCHDOG();
#endif
        if (FLASH_ErasePage( startaddr )!= FLASH_COMPLETE)
            return false;
        startaddr += SOWAYFLASH_PAGE_SIZE;
    }
    __enable_irq();
    FLASH_Lock();
    
    return true;
}



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
bool FLASHOP_WriteData2SpecifiedFlash( u32 startaddr, u32 *psrc, u16 wdlen )
{
    if ((!psrc) || (!wdlen))
        return false;
    
    if (startaddr + wdlen * 4 > CHIPFLASH_MAXIMUM_ADDR)
        return false;
    
    FLASH_Unlock();
    __disable_irq();
    while (wdlen --)
    {
        if (FLASH_ProgramWord( startaddr, *psrc ) != FLASH_COMPLETE)
            return false;
        
        psrc ++;
        startaddr += 4;
    }
    __enable_irq();
    FLASH_Lock();
    
    return true;
}



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
bool FLASHOP_ReadDataFromChipFlash( u8* pdest, u32 op_addr, u32 dwbufsize, u32 dwlen )
{
    u32 val;
    
    if ((!pdest) || (dwbufsize < dwlen))
        return false;
    
    while (dwlen --)
    {
        val = *(u32*)op_addr;
        *pdest ++ = (u8)val;
        *pdest ++ = (u8)(val >> 8);
        *pdest ++ = (u8)(val >> 16);
        *pdest ++ = (u8)(val >> 24);
        op_addr += 4;
    }
    return true;
}



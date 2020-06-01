/*******************************************************************************
                This is the base header file
                    Edited by xieyuyin
*******************************************************************************/
#ifndef _BASEINCLUDES_H
#define _BASEINCLUDES_H

#include    <stdbool.h>
#include    <string.h>
#include    "stm32f0xx.h"
#include    "userdef.h"

#define LITTLEENDIAN    0
#define BIGENDDIAN      1
//Define common macros
#define LOAD_LITTLEENDIAN16(ptr, val)   *(ptr) ++ = (val) & 0xFF; \
                                        *(ptr) ++ = (val) >> 8;
                                            
#define LOAD_BIGENDIAN16(ptr, val)      *(ptr) ++ = (val) >> 8; \
                                        *(ptr) ++ = (val) & 0xFF;
                                            
#define LOAD_LITTLEENDIAN32(ptr, val)   *(ptr) ++ = ((val) & 0xFF); \
                                        *(ptr) ++ = ((val) >> 8) & 0xFF;\
                                        *(ptr) ++ = ((val) >> 16) & 0xFF;\
                                        *(ptr) = (val) >> 24;
                                            
#define LOAD_BIGENDIAN32(ptr, val)      *(ptr) ++ = (val) >> 24; \
                                        *(ptr) ++ = ((val) >> 16) & 0xFF;\
                                        *(ptr) ++ = ((val) >> 8) & 0xFF;\
                                        *(ptr) ++ = ((val) & 0xFF);
                                        
#define GET_LITTLEENDIAN16(val, ptr)    (val) = (*(ptr) ++); \
                                        (val) |= (*(ptr) ++) << 8;
                                        
#define GET_BIGENDIAN16(val, ptr)       (val) = (*(ptr) ++) << 8; \
                                        (val) |= (*(ptr) ++);
                                        
#define GET_LITTLEENDIAN32(val, ptr)    (val) = (*(ptr) ++); \
                                        (val) |= (*(ptr) ++) << 8; \
                                        (val) |= (*(ptr) ++) << 16; \
                                        (val) |= (*(ptr) ++) << 24;
                                        
#define GET_BITENDIAN32(val, ptr)       (val) = (*(ptr) ++) << 24; \
                                        (val) |= (*(ptr) ++) << 16; \
                                        (val) |= (*(ptr) ++) << 8; \
                                        (val) |= *(ptr) ++;

//define watch dog
#define FEED_I_WATCHDOG()  IWDG_ReloadCounter()
                                        
                                        
//Common functions declare
void COMMON_DelayXus( u32 dly );
bool COMMON_Bits8Convert2Bits16( u16 *pdest, u8 *psrc, u8 mode );
bool COMMON_Bits8Convert2Bits32( u32 *pdest, u8 *psrc, u8 mode );
bool COMMON_Bits16Convert2Bits8( u8 *pdest, u16 src, u8 mode );
bool COMMON_Bits32Convert2Bits8( u8 *pdest, u32 src, u8 mode );

bool COMMON_ConverAscii2CommonMode( u8* psrc, u16 datlen);
bool COMMON_Convert1Byteto4Bits(u8* pdest, u8 src);
u8 COMMON_GeneratorLRCCheckCode(u8* psrc, u16 len);
bool COMMON_ConvertHex2Ascii( u8* psrc, u16 len );

//Generator crc check code
bool COMMON_GeneratorU16bitsCRCCheckCode( u16* crcval, u8* psrc, u32 srclen);

#endif


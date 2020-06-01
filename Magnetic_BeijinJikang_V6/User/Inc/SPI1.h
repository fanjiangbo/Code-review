/***************************************************************************************
                    This file is the SPI1 header source file

                Edited by xie_yuyin 2018-05-24 for stm32f072 base version
***************************************************************************************/
#ifndef __SPI1_H
#define __SPI1_H
//Include files
#include    "Baseincludes.h"

//Define macro
#define CPHA_FIRST_EDGE     1
#define CPHA_SECOND_EDGE    2

#define SPI_READ_DUMMY  0xFF


//Functions decalre
void SPI_Spi1DriveInite( void );
void SPI_Spi1SwitchCPHA( u8 val );
bool SPI_Spi1ReadWrit1Byte( u8* val, u8 para );


#endif

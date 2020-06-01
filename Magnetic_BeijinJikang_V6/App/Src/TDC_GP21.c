//This is the TDC GP21 C source file
/***************************************************************************************
                    This file is the TDC GP21 C source file

                Edited by xie_yuyin 2019-06-28 for stm32f072 second edition
***************************************************************************************/
//Include files
#include "SPI1.h"
#include "TDC_GP21.h"

//Local functions declare
static void TDC_ResetTdcChip( void );
static void TDC_CongfigTDCChip( void );
static void TDC_TDCControlPinInit( void );



//Functions implement
/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
void TDC_TDCInit( void )
{
    TDC_TDCControlPinInit(  );  //TDC ic spi_NSS, START, EN_STOP, RESET
    TDC_ResetTdcChip( );
    TDC_CongfigTDCChip( );
}


/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
static void TDC_TDCControlPinInit( void )
{
    GPIO_InitTypeDef GPIO_InitS;
    
    //TDC_NSS pin init
    RCC_AHBPeriphClockCmd( TDC_NSS_RCCID, ENABLE );
    GPIO_InitS.GPIO_Pin = TDC_NSS_PIN;
    GPIO_InitS.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitS.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitS.GPIO_OType = GPIO_OType_PP;
    GPIO_InitS.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(TDC_NSS_PORT, &GPIO_InitS);
    RELEASE_TDC();
    
    //TDC START pin init
    RCC_AHBPeriphClockCmd( TDC_STARTCTRL_RCCID, ENABLE );
    GPIO_InitS.GPIO_Pin = TDC_START_PIN;
    GPIO_InitS.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitS.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitS.GPIO_OType = GPIO_OType_PP;
    GPIO_InitS.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(TDC_START_PORT, &GPIO_InitS);
    DISABLE_START();
    
    //TDC ENSTOP pin init
    RCC_AHBPeriphClockCmd( TDC_STOPCTRL_RCCID, ENABLE );
    GPIO_InitS.GPIO_Pin = TDC_ENSTOP_PIN;
    GPIO_InitS.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitS.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitS.GPIO_OType = GPIO_OType_PP;
    GPIO_InitS.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(TDC_ENSTOP_PORT, &GPIO_InitS);
    DISABLE_ENSTOP();
    
     //Enalbe periph clock
    RCC_AHBPeriphClockCmd( TDC_NRST_RCCID, ENABLE );
    GPIO_InitS.GPIO_Pin = TDC_NRST_PIN;
    GPIO_InitS.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitS.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitS.GPIO_OType = GPIO_OType_PP;
    GPIO_InitS.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(TDC_NRST_PORT, &GPIO_InitS);
    DISABLE_NRST();
}



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
//TDC Chip reset pulse width >= 50 ns
static void TDC_ResetTdcChip( void )
{
    u8 i;
    
    DISABLE_NRST();
    i = 2;
    while (i)
    {
        i --;
    }
    ENABLE_NRST();
    
    i = 2;
    while (i)
    {
        i --;
    }
    DISABLE_NRST();
    
    TDC_WriteCommand2Tdc( CMD_TDC_RESET );
}



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
void TDC_WriteCommand2Tdc( u8 cmd )
{
    u8 tmp;
    
    SPI_Spi1SwitchCPHA( TDC_SPI_MODE );
    SELECT_TDC();
    SPI_Spi1ReadWrit1Byte( &tmp, cmd );
    RELEASE_TDC();
}



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
static void TDC_CongfigTDCChip( void )
{
    TDC_Write_TDCRegister( ADDR_TDCCFG0, 0x008668 );
    TDC_Write_TDCRegister( ADDR_TDCCFG1, 0x214200 );
    TDC_Write_TDCRegister( ADDR_TDCCFG2, 0x400000 );
    TDC_Write_TDCRegister( ADDR_TDCCFG5, 0x080000 );
}



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
void TDC_Write_TDCRegister( u8 regaddr, u32 para )
{
    u8 dummy;
    
    SPI_Spi1SwitchCPHA( TDC_SPI_MODE );
    
    SELECT_TDC();
    SPI_Spi1ReadWrit1Byte( &dummy, regaddr );
    SPI_Spi1ReadWrit1Byte( &dummy, (u8)(para >> 16) );
    SPI_Spi1ReadWrit1Byte( &dummy, (u8)(para >> 8) );
    SPI_Spi1ReadWrit1Byte( &dummy, (u8)(para) );
    
    RELEASE_TDC();
}



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
u32 TDC_Read32BitsDataFromTDC( u8 destaddr )
{
    u32 val;
    u8 tmp;
    
    SPI_Spi1SwitchCPHA( TDC_SPI_MODE );
    SELECT_TDC();
    //Write read data dest address
    SPI_Spi1ReadWrit1Byte( &tmp, destaddr );
    
    //Read data
    SPI_Spi1ReadWrit1Byte( &tmp, SPI_READ_DUMMY );
    val = tmp << 24;
    SPI_Spi1ReadWrit1Byte( &tmp, SPI_READ_DUMMY );
    val |= tmp << 16;
    SPI_Spi1ReadWrit1Byte( &tmp, SPI_READ_DUMMY );
    val |= tmp << 8;
    SPI_Spi1ReadWrit1Byte( &tmp, SPI_READ_DUMMY );
    val |= tmp;
    
    RELEASE_TDC();
    return val;
}



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/

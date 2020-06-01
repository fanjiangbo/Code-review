/***************************************************************************************



***************************************************************************************/
//Inlcudes
#include "Analog.h"

//Variables define

//Local functions declare
static void ANALOG_WriteCommand2AD5422( u8 addr, u16 para );
static void ANALOG_Write16Bits2AD5422( u8 destaddr, u16 dat );


//Functions implement
/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
void ANALOG_ChipInit( void )
{
    GPIO_InitTypeDef   GPIO_InitS;
    
    RCC_AHBPeriphClockCmd( ADOUT_CTRL_RCCID, ENABLE );
    
    //AD5422 C1 Init
    GPIO_InitS.GPIO_Pin = ADOUT_CHANNEL1_PIN;
    GPIO_InitS.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitS.GPIO_OType = GPIO_OType_PP;
    GPIO_InitS.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ADOUT_CHANNEL1_PORT, &GPIO_InitS);
    ADOUT_CHANNEL1_RELEASE();
    
    //ADG779
    RCC_AHBPeriphClockCmd( ADG779_CTRL_RCCID, ENABLE );
    //ADG79 Ctrol pin init
    GPIO_InitS.GPIO_Pin = ADG999_CTRL_PIN;
    GPIO_InitS.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitS.GPIO_OType = GPIO_OType_PP;
    GPIO_InitS.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ADG779_CTRL_PORT, &GPIO_InitS);
}


/***************************************************************************************
Name:  
Func:   
Para:
Retn:
***************************************************************************************/
void ANALOG_AD5422OutPutParametersConfig( u8 rangeid )
{
    if ((rangeid < ANALOG_OUTPUT_MAXNUM) && (rangeid))
    {
        ANALOG_WriteCommand2AD5422( ADDR_AD5422_RESET, 1 );
        COMMON_DelayXus( 40 );
    }
    else
        return;
    
    switch (rangeid)
    {
    case ANALOG_VOL_NEGA5TO5:
        ANALOG_WriteCommand2AD5422( ADDR_AD5422_CTRL, (AD5422OUTEN | AD5422RGSEL_2) );
        break;
        
    case ANALOG_VOL_NEGA10TO10:
        ANALOG_WriteCommand2AD5422( ADDR_AD5422_CTRL, (AD5422OUTEN | AD5422RGSEL_3) );
        break;
        
    case ANALOG_VOL_ZEROTO5:
        ANALOG_WriteCommand2AD5422( ADDR_AD5422_CTRL, (AD5422OUTEN | AD5422RGSEL_0) );
        break;
        
    case ANALOG_VOL_ZEROTO10:
        ANALOG_WriteCommand2AD5422( ADDR_AD5422_CTRL, (AD5422OUTEN | AD5422RGSEL_1) );
        break;
        
    case ANALOG_CUR_FOURTO20:
        ANALOG_WriteCommand2AD5422( ADDR_AD5422_CTRL, (AD5422OUTEN | AD5422RGSEL_5) );
        break;
        
    case ANALOG_CUR_ZEROTO20:
        ANALOG_WriteCommand2AD5422( ADDR_AD5422_CTRL, (AD5422OUTEN | AD5422RGSEL_6) );
        break;
        
    default:
        break;
    }
}



/***************************************************************************************
Name:  
Func:   
Para:
Retn:
***************************************************************************************/
static void ANALOG_WriteCommand2AD5422( u8 addr, u16 para )
{
    SPI_Spi1SwitchCPHA( AD5422_SPI_MODE );
    
    ADOUT_CHANNEL1_SELECT();
    ANALOG_Write16Bits2AD5422(addr, para);
    ADOUT_CHANNEL1_RELEASE();
}

    
/***************************************************************************************
Name:  
Func:   
Para:
Retn:
***************************************************************************************/
static void ANALOG_Write16Bits2AD5422( u8 destaddr, u16 dat )
{
    u8 dummy;
    
    SPI_Spi1ReadWrit1Byte( &dummy, destaddr );
    SPI_Spi1ReadWrit1Byte( &dummy, (u8)(dat >> 8) );
    SPI_Spi1ReadWrit1Byte( &dummy, (u8)(dat) );
}
        
    
/***************************************************************************************
Name:  
Func:   
Para:
Retn:
***************************************************************************************/
void ANALOG_RefreshAD5422Output( u32 dat, u8 chnid )
{
    SPI_Spi1SwitchCPHA( AD5422_SPI_MODE );
    
    if (chnid == 1)
    {
        ADOUT_CHANNEL1_SELECT();
        ANALOG_Write16Bits2AD5422(ADDR_AD5422_DATA, dat);
        ADOUT_CHANNEL1_RELEASE();
    }
}

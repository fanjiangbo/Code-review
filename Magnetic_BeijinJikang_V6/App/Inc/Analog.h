/***************************************************************************************



***************************************************************************************/
#ifndef __ANALOG_H
#define __ANALOG_H

//Inlcudes
#include "SPI1.h"
#include "Baseincludes.h"

//Macro define
#define ADG779_CTRL_RCCID   RCC_AHBPeriph_GPIOB
#define ADOUT_CTRL_RCCID    RCC_AHBPeriph_GPIOA

#define ADOUT_CHANNEL1_PORT     GPIOA
#define ADOUT_CHANNEL1_PIN      GPIO_Pin_6
#define ADOUT_CHANNEL1_SELECT() GPIO_ResetBits(ADOUT_CHANNEL1_PORT, ADOUT_CHANNEL1_PIN)  
#define ADOUT_CHANNEL1_RELEASE()    GPIO_SetBits(ADOUT_CHANNEL1_PORT, ADOUT_CHANNEL1_PIN)

#define ADG779_CTRL_PORT    GPIOB
#define ADG999_CTRL_PIN     GPIO_Pin_11
#define ADG779_DISABLE()    GPIO_ResetBits(ADG779_CTRL_PORT, ADG999_CTRL_PIN)
#define ADG779_ENABLE()     GPIO_SetBits(ADG779_CTRL_PORT, ADG999_CTRL_PIN)

//AD5422 spi CPHA mode
#define AD5422_SPI_MODE         CPHA_FIRST_EDGE

//AD5422 output range define
enum{
    ANALOG_VOL_NEGA5TO5 =   1,       //-5~5V
    ANALOG_VOL_NEGA10TO10 = 2,       //-10~10V
    ANALOG_VOL_ZEROTO5 =    3,       //0~5V
    ANALOG_VOL_ZEROTO10 =   4,       //0~10V
    ANALOG_CUR_FOURTO20 =   5,       //4~20mA
    ANALOG_CUR_ZEROTO20 =   6,       //0~20mA
    ANALOG_OUTPUT_MAXNUM
};

//AD5422 address
#define ADDR_AD5422_DATA    0x01    //data register adress    
#define ADDR_AD5422_CTRL    0x55    //control register adress
#define ADDR_AD5422_RESET   0x56    //reset register adress

#define AD5422RGSEL_0   	    0x00    	    //0~5V 
#define AD5422RGSEL_1   	    0x01            //0~10V 
#define AD5422RGSEL_2   	    0x02            //-5~5V 
#define AD5422RGSEL_3   	    0x03            //-10~10V 
#define AD5422RGSEL_5   	    0x05            //4~20mA 
#define AD5422RGSEL_6   	    0x06            //0~20mA 
#define AD5422RGSEL_7   	    0x07            //0~24mA 
#define AD5422DCEN      	    0x08            //Daisy chain enable
#define AD5422SREN      	    0x10            //Data conversion speed control enable 
#define AD5422OUTEN     	    0x1000          //enable output
#define AD5422REXT      	    0x2000          //Peripheral resistance. Used to reduce the effect of temperature on the DA output current
#define AD5422OVRRNG    	    0x4000          //Increase the voltage output by 10%
#define AD5422CLRSEL    	    0x8000          //Output clear bit¡ê?0:output 0V; 1:output median value

//Functions declare
void ANALOG_ChipInit( void );
void ANALOG_AD5422OutPutParametersConfig( u8 range );
void ANALOG_RefreshAD5422Output( u32 dat, u8 chnid );

//Interrupt functions


#endif
/***************************************************************************************


***************************************************************************************/
#ifndef __TDC_GP21_H
#define __TDC_GP21_H

//Include files
#include    "Baseincludes.h"

//TDC SPI edge type
#define TDC_SPI_MODE    CPHA_SECOND_EDGE

//TDC command
#define CMD_TDC_RESET   0x50
#define CMD_TDC_INIT    0x70

//TDC Register address
#define ADDR_TDCCFG0    0x80
#define ADDR_TDCCFG1    0x81
#define ADDR_TDCCFG2    0x82
#define ADDR_TDCCFG3    0x83
#define ADDR_TDCCFG4    0x84
#define ADDR_TDCCFG5    0x85

#define ADDR_TDCDATA0   0xB0
#define ADDR_TDCDATA1   0xB1
#define ADDR_TDCDATA2   0xB2
#define ADDR_TDCDATA3   0xB3
#define ADDR_TDC_START  0xB4

//TDC SPI NSS
//TDC spi NSS pin
#define TDC_NSS_RCCID   RCC_AHBPeriph_GPIOA
#define TDC_NSS_PORT    GPIOA
#define TDC_NSS_PIN     GPIO_Pin_15
#define SELECT_TDC()    GPIO_ResetBits(TDC_NSS_PORT, TDC_NSS_PIN)
#define RELEASE_TDC()   GPIO_SetBits(TDC_NSS_PORT, TDC_NSS_PIN)

//TDC STOP control port and pin
#define TDC_STOPCTRL_RCCID  RCC_AHBPeriph_GPIOA
#define TDC_ENSTOP_PORT     GPIOA
#define TDC_ENSTOP_PIN      GPIO_Pin_1
#define ENABLE_ENSTOP()     GPIO_SetBits(TDC_ENSTOP_PORT, TDC_ENSTOP_PIN)
#define DISABLE_ENSTOP()    GPIO_ResetBits(TDC_ENSTOP_PORT, TDC_ENSTOP_PIN)

//TDC START control port and pin
#define TDC_STARTCTRL_RCCID RCC_AHBPeriph_GPIOC
#define TDC_START_PORT      GPIOC
#define TDC_START_PIN       GPIO_Pin_13
#define ENABLE_START()      GPIO_SetBits(TDC_START_PORT, TDC_START_PIN)
#define DISABLE_START()     GPIO_ResetBits(TDC_START_PORT, TDC_START_PIN)

//TDC NRST port and pin
#define TDC_NRST_RCCID  RCC_AHBPeriph_GPIOB
#define TDC_NRST_PORT   GPIOB
#define TDC_NRST_PIN    GPIO_Pin_9
#define ENABLE_NRST()   GPIO_ResetBits(TDC_NRST_PORT, TDC_NRST_PIN)
#define DISABLE_NRST()  GPIO_SetBits(TDC_NRST_PORT, TDC_NRST_PIN)

//ANA Stop exti interrupt parameter
#define ANA_STOP_CLK_RCC    RCC_AHBPeriph_GPIOA
#define ANA_STOP_SRC_PORT   GPIOA
#define ANA_STOP_SRC_PIN    GPIO_Pin_0
#define ANA_STOP_EXTI_LINE  EXTI_Line0
#define ANA_STOP_EXTI_CHNL  EXTI0_1_IRQn
#define ANA_STOP_EXTI_ENABLE()  (EXTI->IMR |= ANA_STOP_EXTI_LINE)
#define ANA_STOP_EXTI_DISABLE() (EXTI->IMR &= ~ANA_STOP_EXTI_LINE)

//TDC INTN exti interrupt parameter
#define TDC_INTN_CLK_RCC        RCC_AHBPeriph_GPIOB
#define TDC_INTN_SRC_PORT       GPIOB
#define TDC_INTN_SRC_PIN        GPIO_Pin_8
#define TDC_INTN_EXTI_LINE      EXTI_Line8
#define TDC_INTN_EXTI_PORTSRC   EXTI_PortSourceGPIOB
#define TDC_INTN_TXTI_PINSRC    EXTI_PinSource8
#define TDC_INTN_EXTI_CHNL      EXTI4_15_IRQn
#define TDC_INTN_EXTI_ENABLE()  (EXTI->IMR |= TDC_INTN_EXTI_LINE)
#define TDC_INTN_EXTI_DISABLE() (EXTI->IMR &= ~TDC_INTN_EXTI_LINE)


//Functions declare
void TDC_TDCInit( void );
void TDC_WriteCommand2Tdc( u8 cmd );
void TDC_WriteCommand2Tdc( u8 cmd );
void TDC_Write_TDCRegister( u8 regaddr, u32 para );
u32 TDC_Read32BitsDataFromTDC( u8 destaddr );

#endif

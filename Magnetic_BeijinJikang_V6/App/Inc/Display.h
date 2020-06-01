/***************************************************************************************


***************************************************************************************/
#ifndef __DISPLAY_H
#define __DISPLAY_H


//Include files
#include    "Baseincludes.h"

//Led macro
#define LED_PORT        GPIOB
#define LED_GRN_PIN     GPIO_Pin_15
#define LED_RED_PIN     GPIO_Pin_14
#define LED_RED_OFF()   GPIO_ResetBits( LED_PORT, LED_RED_PIN )
#define LED_RED_ON()    GPIO_SetBits( LED_PORT, LED_RED_PIN )
#define LED_GRN_OFF()   GPIO_ResetBits( LED_PORT, LED_GRN_PIN )
#define LED_GRN_ON()    GPIO_SetBits( LED_PORT, LED_GRN_PIN )

//Functions declare
void DISPLAY_LedInit( u8* ppara );


#endif



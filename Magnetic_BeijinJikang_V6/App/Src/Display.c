/***************************************************************************************


***************************************************************************************/
//Include files
#include "Display.h"
#include "STM32FX_Systick.h"

//Variables define
static u8 redledflg = 0;
static bool grnledflg = true;

//Local functions declare
static void DISPLAY_LedControlGpioInit( void );
static bool DISPLAY_LedBlinkProcess( void* pdat );


/*******************************************************************************
Name:   
Func:   
Para:   
Retn:   
*******************************************************************************/
void DISPLAY_LedInit( u8* ppara )
{
    DISPLAY_LedControlGpioInit();
    
    //Set LED blink timer
    STM32FX_Systick_TimerSet( eTmr_LedBlinkTimer, DISPLAY_LedBlinkProcess, (void*)ppara, 100 );
}

/*******************************************************************************
Name:   
Func:   
Para:   
Retn:   
*******************************************************************************/
static void DISPLAY_LedControlGpioInit( void )
{
    GPIO_InitTypeDef   GPIO_InitStructure;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = LED_RED_PIN | LED_GRN_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init( LED_PORT, &GPIO_InitStructure );
    LED_RED_ON();
    LED_GRN_ON();
}


/*******************************************************************************
Name:   
Func:   
Para:   
Retn:   
*******************************************************************************/
static bool DISPLAY_LedBlinkProcess( void* pdat )
{
    u8 redled;
    u8* ppara;
    
    ppara = (u8*)pdat;
    redled = *ppara;
    
    if (!redled)    //Magnet ring normal
    {
        LED_RED_OFF();
    }
    else if(redled == 1)   //Head dead area
    {
        redledflg ++;
        if (redledflg == 2)
        {
            LED_RED_OFF();
        }
        else if (redledflg == 4)
        {
            redledflg = 0;
            LED_RED_ON();
        }
    }
    else if (redled == 2)
    {
        if (!redledflg)
        {
            redledflg = 1;
            LED_RED_OFF();
        }
        else
        {
            redledflg = 0;
            LED_RED_ON();
        }
    }
    else if (redled == 3)
    {
        LED_RED_ON();
    }
    
    
    //Reset magnet status
    *ppara = 3;
    
    //Communicate status display
    ppara ++;
    switch (*ppara)
    {
    case 0:
        break;
        
    case 8:
        *ppara = 0;
        LED_GRN_ON();
        grnledflg = true;
        break;
        
    default:
        if (grnledflg)
            LED_GRN_OFF();
        else
            LED_GRN_ON();
        
        grnledflg = !grnledflg;
        (*ppara) += 1;
        break;
    }
    
    return true;
}

/***************************************************************************************
            This file is the STM32F37X Systick header file
***************************************************************************************/
#ifndef __STM32FX_SYSTICK_H
#define __STM32FX_SYSTICK_H

//Inlcude files
#include "Baseincludes.h"

//Datatype define
enum{
    TimerNoUse = 0,
    TimerUsing
};

typedef enum{
    eTmr_BootLoader = 0,
    //Todo: Add user timer id start with eTmr_
    eTmr_ModbusTimer,
    eTmr_LedBlinkTimer,
    
    //user timer id
    eTmr_CountId
}E_TMR_TAG;

typedef bool (*TmrCallBack)(void *);
typedef struct{
    u8          mState; //Current id timer statu 0:stopped  1:running
    u8          mFlag;  //Timer timeout flag 1:time out
    u32         delay;  //Delay time value
    u32         remain; //Remain delay value
    void*       pdata;  //User parameter
    TmrCallBack pFCB;   //Call back function pointer
}S_TMR_INFO, *PS_TMR_INFO;

//Functions declare
void STM32FX_Systick_TickClockConfig( void );
void STM32FX_Systick_Delayxms( u32 ms );
void STM32FX_Systick_TimerProcess( void );
void STM32FX_Systick_TimerKill( u32 i );
void STM32FX_Systick_TimerSet( u8 tmrid, TmrCallBack pcb, void* pdat, u32 dlyms );

void SysTick_Handler(void);

#endif

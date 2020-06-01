/***************************************************************************************
                                This file is the STM32FXX
                        Systick functions implement source file
                Edited by Xie yuyin 2019-09-18 the first edtion, Soway Ltd
***************************************************************************************/
//Inlcude files
#include "STM32FX_Systick.h"


//Variables define
static u32 delaymscnt;
static S_TMR_INFO TmrTbl[eTmr_CountId];


//Local functions declare
static void STM32FX_Systick_TimerControlInit( void );

//Functions implement
/***************************************************************************************
Name:   
Func:   
Para:   
Retn:  
***************************************************************************************/
void STM32FX_Systick_TickClockConfig( void )
{
    __disable_irq();    //Disable all interrupt
    if (SysTick_Config(SystemCoreClock / 1000))
    {
        /* Capture error */ 
        while (1);
    }
    __enable_irq();     //Enable all interrupt
    
    //Inite timer
    STM32FX_Systick_TimerControlInit();
}


/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
void STM32FX_Systick_Delayxms( u32 ms )
{
    delaymscnt = ms;
    
    while (delaymscnt)
    {
        //Enalbe watchdog
#ifdef USE_IDWDG
            FEED_I_WATCHDOG();
#endif
    }
}



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
void STM32FX_Systick_TimerProcess( void )
{
    u32 i;
    for (i = 0; i < eTmr_CountId; i ++)
    {
        if (TmrTbl[i].mFlag)
        {
            TmrTbl[i].mFlag = 0;
            
            if ((TmrTbl[i].pFCB)(TmrTbl[i].pdata))
            {
                TmrTbl[i].remain = TmrTbl[i].delay;
                TmrTbl[i].mState = TimerUsing;
            }
        }
    }
}



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
static void STM32FX_Systick_TimerControlInit( void )
{
    u32 i;
    for (i = 0; i < eTmr_CountId; i ++)
    {
        TmrTbl[i].delay = 0;
        TmrTbl[i].remain = 0;
        TmrTbl[i].mFlag = 0;
        TmrTbl[i].mState = TimerNoUse;
        TmrTbl[i].pFCB = NULL;
        TmrTbl[i].pdata = NULL;
    }
}


/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
void STM32FX_Systick_TimerSet( u8 tmrid, TmrCallBack pcb, void* pdat, u32 dlyms )
{
    if ((tmrid >= eTmr_CountId) || (!pcb) || (!dlyms))
        return;
    
    TmrTbl[tmrid].mState = TimerNoUse;
    
    TmrTbl[tmrid].pFCB = pcb;
    TmrTbl[tmrid].pdata = pdat;
    TmrTbl[tmrid].delay = dlyms;
    TmrTbl[tmrid].remain = dlyms;
    TmrTbl[tmrid].mState = TimerUsing;
}


/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
void STM32FX_Systick_TimerKill( u32 i )
{
    TmrTbl[i].delay = 0;
    TmrTbl[i].remain = 0;
    TmrTbl[i].mFlag = 0;
    TmrTbl[i].mState = TimerNoUse;
    TmrTbl[i].pFCB = NULL;
    TmrTbl[i].pdata = NULL;
}


/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
void SysTick_Handler(void)
{
    //Delay x ms
    if (delaymscnt)
    {
        delaymscnt --;
    }
    
    //Timer
    u32 i;
    for (i = 0; i < eTmr_CountId; i ++)
    {
        if (TmrTbl[i].mState == TimerUsing)
        {
            if (TmrTbl[i].remain)
            {
                TmrTbl[i].remain --;
            }
            else
            {
                TmrTbl[i].mFlag = 1;
                TmrTbl[i].mState = TimerNoUse;
            }
        }
    }
}

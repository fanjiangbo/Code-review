/***************************************************************************************
            This file is the Modbus Lib Test main C source file
            
            Include Test sub_function test all modbus funtions
***************************************************************************************/
//Include files
#include "flashop.h"
#include "STM32FX_Systick.h"
#include "UserApp.h"

//Local functions declare
#ifdef USE_IDWDG
static void IWDG_Init(void);
#endif

#ifdef IAP_REMAP_INT
//Remap function declare
void RemapInterruptVerctor( void );

//Remap address define
#define APPLICATION_ADDRESS     (uint32_t)SOWAYAPP_START_ADDR

//Define variable
    #if   (defined ( __CC_ARM ))
    __IO uint32_t VectorTable[48] __attribute__((at(0x20000000)));
    #elif (defined (__ICCARM__))
    #pragma location = 0x20000000
    __no_init __IO uint32_t VectorTable[48];
    #elif defined   (  __GNUC__  )
    __IO uint32_t VectorTable[48] __attribute__((section(".RAMVectorTable")));
    #elif defined ( __TASKING__ )
    __IO uint32_t VectorTable[48] __at(0x20000000);
    #endif
//Remap function implement
/*******************************************************************************
Name:   
Func:   
Para:   
Retn:   
*******************************************************************************/
void RemapInterruptVerctor( void )
{
    for(u32 i = 0; i < 48; i++)
    {
        VectorTable[i] = *(__IO uint32_t*)(APPLICATION_ADDRESS + (i<<2));
    }
	
    /* Enable the SYSCFG peripheral clock*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE); 
    /* Remap SRAM at 0x00000000 */
    SYSCFG_MemoryRemapConfig(SYSCFG_MemoryRemap_SRAM);
}
#endif



/*******************************************************************************
Name:   
Func:   
Para:   
Retn:   
*******************************************************************************/
void main( void )
{
#ifdef IAP_REMAP_INT
    RemapInterruptVerctor();
#endif
    
    //Schedule init
    STM32FX_Systick_TickClockConfig( );
    
    //Todo: App init
    USERApp_Init();

#ifdef USE_IDWDG
    //Init watchdog
    IWDG_Init();
#endif
    
    //Start process Magnet
    USERApp_MagnetProcess();
}

#ifdef USE_IDWDG
/***************************************************************************************
Name:  
Func:   
Para:
Retn:
***************************************************************************************/
static void IWDG_Init(void)
{
    
    RCC_LSICmd(ENABLE); /*!< LSI Enable */
    while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);    /*!< Wait till LSI is ready */

    /* IWDG timeout equal to 250 ms (the timeout may varies due to LSI frequencydispersion) */
    /* Enable write access to IWDG_PR and IWDG_RLR registers */
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    /* IWDG counter clock: LSI/256 */
    IWDG_SetPrescaler(IWDG_Prescaler_256);   // 40K / 256 = 156.25 -> 6.4ms per tick

    /* Set counter reload value*/
    IWDG_SetReload(1500);         // 6.4ms * 1500 = 9.6s

    /* Reload IWDG counter */
    IWDG_ReloadCounter();

    /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
    IWDG_Enable();
}
#endif



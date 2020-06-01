/***************************************************************************************
                              This file is the STM32F3X
                        modbus functions implement head file
                Edited by Xie yuyin 2019-09-18 the first edtion, Soway Co., Ltd
***************************************************************************************/
#ifndef __STM32FXMODBUSLIB_H
#define __STM32FXMODBUSLIB_H

#define STM32FX_MODBUS_LIBVER "STM32FX_LibVer:01.00.000"
//Include files
#include "Baseincludes.h"

//Define Communicate UART parameter
//#define MODBUS_RTU      0
//#define MODBUS_ASCII    7
//Define Communicate UART parameter
#define MODBUS_RTU      1
#define MODBUS_ASCII    2

//Define function code units(byte)
#define COILADDRLEN 2
#define FUNC03UNITS 2
#define FUNC04UNITS 4
#define FUNC26UNITS 4
#define FUNC2A_EXPT 6
#define FUNC2ALEN   (1 << FUNC2A_EXPT)  //This value forbiden modify


//Define modbus logic value
#define MODBUS_LOGIC_TRUE   0xFF00
#define MODBUS_LOGIC_FALSE  0x0000
#define SOWAY_LOGIC_TRUE    0xF0
#define SOWAY_LOGIC_FALSE   0x0F

//Modbus ASCII message frame header and tail flag
#define ASCII_FRAME_HEADER  0x3A
#define ASCII_FRAME_ENDFLAG 0x0D0A


typedef struct 
{
	u8 SlaveAddr;
	u8 BaudRate;
    u8 Parity;
    u8 ModbusType;  //1-RTU 2-ASCII
}S_COMM_PARA,*PS_COMM_PARA;

typedef struct{
    u8* prcv;
    u16 rcvsize;
    u8* psnd;
    u16 sndsize;
    u8* pparse;
    u16 parsesize;
}S_COMM_BUFPARA, *PS_COMM_BUFPARA;

//Function code data type
typedef struct{
    u16*        pStVal;
    const u16*  pAddrTab;
    u16         paranum;
}S_F03_EX_PARA, *PS_F03_EX_PARA;    //EX_exchange data

typedef struct{
    u32*        pStVal;
    const u16*  pAddrTab;
    u16         paranum;
}S_F04_EX_PARA, *PS_F04_EX_PARA;

typedef struct{
    u32*        pStVal;
    const u16*  pAddrTab;
    u16         paranum;
}S_F26_EX_PARA, *PS_F26_EX_PARA;

typedef struct{
    u8*         pStVal;
    const u16*  pAddrTab;
    u16         paranum;
}S_F05_EX_PARA, *PS_F05_EX_PARA;

typedef struct{
    u8*         pStVal;
    const u16*  pAddrTab;
    u16         paranum;
}S_F25_EX_PARA, *PS_F25_EX_PARA;

typedef struct{
    const char* pString;
    const u16*  pAddrTab;
    u16         paranum;
}S_F2A_EX_PARA, *PS_F2A_EX_PARA;


//Define
#define MBCSET_SLAVER_ID    0x01
#define MBCSET_MODBUSTYPE   0x02
#define MBCSET_BAUDRATE     0x04
#define MBCSET_PARITY       0x08
#define MBCSET_ALLPARA      0x80
#define MBCSET_PARA_NUM     0x05

//------------------------define modbus parse function type----------------------------
typedef void (*ModbusParseFunc)(u8 *psrc, u32 len, u8 type );


//--------------------------define modbus communicate result---------------------------
typedef enum{
    ModbusOperatorSucceed = 0,  //Modbus operator succeed
    FuncCodeNotSupport = 1,     //Function code or Subfunction code not support
    StartAddrOrRegOffsetErr = 2,//Start address or start address plus register number error
    RegisterNumberError = 3,    //Function code corresponding Register number error
    FuncOperationFailed = 4,    //Function code operator error
    ReadDatBufferSizeError = 5, //Read data buffer size error
    ReadDatItemNotSupport = 6,  //Read data item not support
    WriteDatItemNotSUPPORT = 7, //Write data item not support
    
    WriteParameterFailed = 8,  
    OperatorPraraError = 9,
    MsgFrameFormateError = 10,
    GetSoftwareVerError = 11,
    Func05OperaterFailed = 12,
    
    ModbusCommunicateErr
}Modbus_Result;

//Function code list
#define FUNC03_READ     0x03
#define FUNC04_READ     0x04
#define FUNC05_WRITE    0x05
#define FUNC10_WRITE    0x10
#define FUNC25_WRITE    0x25
#define FUNC26_READ     0x26
#define FUNC27_WRITE    0x27
#define FUNC2A_WRITE    0x2A
#define FUNC2B_READ     0x2B
#define FUNC41_UPGRADE  0x41

//RS485 communicate control port and pin
#define RS485_RX_CTRL_PORT  GPIOA
#define RS485_RX_CTRL_PIN   GPIO_Pin_8
#define RS485_RCC_SOURCE    RCC_AHBPeriph_GPIOA
#define RS485_TX_ENABLE()   GPIO_SetBits(RS485_RX_CTRL_PORT, RS485_RX_CTRL_PIN)
#define RS485_RX_ENABLE()   GPIO_ResetBits(RS485_RX_CTRL_PORT, RS485_RX_CTRL_PIN)

//Old version Modbus control port and pin
//#define RS485_RX_CTRL_PORT  GPIOB
//#define RS485_RX_CTRL_PIN   GPIO_Pin_2
//#define RS485_RCC_SOURCE    RCC_AHBPeriph_GPIOB

//------------------Usart1 port and pin, and baudrate-----------------------------------
//Define communicate usart Tx and Rx
#define	MODBUS_USARTNUM     USART1
#define	MODBUS_TXD_PORT     GPIOA
#define	MODBUS_TXD_PIN      GPIO_Pin_9
#define	MODBUS_RXD_PORT     GPIOA
#define	MODBUS_RXD_PIN      GPIO_Pin_10
#define MODBUS_PORT_RCC     RCC_AHBPeriph_GPIOA

#define MODBUS_AFSOURCEPORT GPIOA
#define MODBUS_TXD_AFSOURCE GPIO_PinSource9
#define MODBUS_RXD_AFSOURCE GPIO_PinSource10

//Define commnicate parameters
#define	MODBUS_USARTPRIO	2
#define MODBUS_USARTIRQN	USART1_IRQn
#define MODBUS_USART_RCC	RCC_APB2Periph_USART1


//Define pc communicate DMA channel
#define MODBUSTXD_DMA_IRQN      DMA1_Channel2_3_IRQn
#define MODBUSTXD_DMA_PRIO      3
#define MODBUSTXD_DMA_INTFLAG   DMA1_FLAG_GL2
#define MODBUSTXD_DMA_CHANNEL   DMA1_Channel2


#define MODBUSRXD_DMA_CHANNEL   DMA1_Channel3

//Functions declare
bool STM32FX_Modbus_CommunicateInit( PS_COMM_BUFPARA pbufpara, ModbusParseFunc pFunc );
void STM32FX_Modbus_CommunicateCommParaConfig( PS_COMM_PARA p_compara );
void STM32FX_Modbus_CommunicateSendData( u16 len );
bool STM32FX_Modbus_ModbusIsSendingData( void );
void STM32FX_Modbus_UpgradeResponseProcess( u8* psrc, u16 len );

//Command functions
bool STM32FX_Modbus_FunctionCode03ParameterInit( PS_F03_EX_PARA pdat );
bool STM32FX_Modbus_FunctionCode04ParameterInit( PS_F04_EX_PARA pdat );
bool STM32FX_Modbus_FunctionCode26ParameterInit( PS_F26_EX_PARA pdat );
bool STM32FX_Modbus_FunctionCode05ParameterInit( PS_F05_EX_PARA pdat );
bool STM32FX_Modbus_FunctionCode25ParameterInit( PS_F25_EX_PARA pdat );
bool STM32FX_Modbus_FunctionCode2AParameterInit( PS_F2A_EX_PARA pdat );
Modbus_Result STM32FX_Modbus_FunctionCodeReadData( u8* psrc );
Modbus_Result STM32FX_Modbus_FunctionCode05_25WriteData( u8* psrc );
void STM32FX_Modbus_WriteParameterFunctionResponse( u8* psrc, Modbus_Result res );
Modbus_Result STM32FX_Modbus_FunctionCode10_27WriteData( u8* psrc );
Modbus_Result STM32FX_Modbus_FunctionCode05_25WriteData( u8* psrc );
Modbus_Result STM32FX_Modbus_FunctionCode2AWriteInformation( u8* psrc );

//Interrupt enter functions
void TIM7_IRQHandler( void );
void USART1_IRQHandler( void );
void DMA1_Channel2_3_IRQHandler( void );

#endif

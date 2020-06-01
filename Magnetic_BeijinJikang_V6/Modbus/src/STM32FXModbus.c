/***************************************************************************************
                                This file is the STM32FX
                        Usart1 functions implement source file
                Edited by Xie yuyin 2019-09-18 the first edtion, Soway Co., Ltd
***************************************************************************************/
//Include files
#include "STM32FX_Systick.h"
#include "stm32fxmodbuslib.h"

//Communicate depart define
#define MODBUS_RTU_DLY      3
#define MODBUS_ASCII_DLY    20
#define MODBUS_CMD_ERR_MASK 0x80

//Datatype define
//---------------Queue datatype structure define------------------------
typedef struct{
	u16	head;	//queue front
	u16	tail;	//queue rear(tail)
	u16	size;	//queue size
	u8*	pqbuf;	//queue pointer
}S_QUEUE_TAG, *PS_QUEUE_TAG;

//--------------define send data structure------------------------------
typedef struct{
    u8* psndbuf;
    u16 bufsize;
}S_SNDDAT_TAG,*PS_SNDDAT_TAG;

//--------------define parse structure----------------------------------
typedef struct{
    u8*             pbuf;
    u32             bufsize;
    ModbusParseFunc pparse;
}S_PARSE_TAG, *PS_PARSE_TAG;

static union{
    u8 flgbyte;
    struct{
        u8 flgtransing: 1;
        u8 flgtestbit:  1;
        u8 flgreserve:  6;
    }flgbits;
}u_flag;

//Variables define
static u8 modbus_idle_int_flg = 1;
static u16 DMACurDataCounter;

static S_COMM_PARA s_compara = { 0x01, 0x03, 0x03, 0x01 };  //Slv_addr 9600 NO_Parity RTU
static S_QUEUE_TAG s_mdbs_rcv;
static S_SNDDAT_TAG s_mdbs_snd;
static S_PARSE_TAG s_mdbs_parse;

//Modbus function code variables
static S_F03_EX_PARA s_f03; //f10
static S_F04_EX_PARA s_f04;
static S_F05_EX_PARA s_f05;
static S_F26_EX_PARA s_f26; //f27
static S_F25_EX_PARA s_f25;
static S_F2A_EX_PARA s_f2A; //f2B

//Local functions declare
static void STM32FX_Modbus_Timer7Config( void );
static bool STM32FX_Modbus_ModbusTimer( void* pdat );
static void STM32FX_Modbus_CommunicateDMAConfig( void );
static void STM32FX_Modbus_CommunicatePortPinAndIntInit( void );
static void STM32FX_Modbus_CommunicateResponseInformation( u16 datlen );

static Modbus_Result STM32FX_Modbus_FunctionCode03Process( u16 regnum, u16 star_addr );
static Modbus_Result STM32FX_Modbus_FunctionCode04Process( u16 reg_num, u16 star_addr );
static Modbus_Result STM32FX_Modbus_FunctionCode26Process( u16 reg_num, u16 star_addr );
static Modbus_Result STM32FX_Modbus_FunctionCode2BProcess( u16 reg_num, u16* star_addr );
static Modbus_Result STM32FX_Modbus_FunctionCode10Process( u8* psrc, u16 reg_num, u16 star_addr );
static Modbus_Result STM32FX_Modbus_FunctionCode27Process( u8* psrc, u16 reg_num, u16 star_addr );


//Functions implement
/***************************************************************************************
Name:   
Func:   
Para:   
Retn:  
***************************************************************************************/
#define ASCII_FRAME_HEADER  0x3A
#define ASCII_FRAME_ENTER   0x0D
#define ASCII_FRAME_NEWLINE 0x0A
#define ASCII_FRAME_ENDFLAG 0x0D0A
static bool STM32FX_Modbus_ModbusTimer( void* pdat )
{
    u32 len;
    u8* ptmp;
    u16 crc_chk;
    u16 tmp;
    
    //Parse parameter not init
    if (!s_mdbs_parse.pbuf)
        return false;
    
    s_mdbs_rcv.tail =s_mdbs_rcv.size - DMACurDataCounter;
    memset( s_mdbs_parse.pbuf, 0, s_mdbs_parse.bufsize );
    
    len = 0;
    while (s_mdbs_rcv.head != s_mdbs_rcv.tail)
    {
#ifdef USE_IDWDG
            FEED_I_WATCHDOG();
#endif
        //Read data
        s_mdbs_parse.pbuf[len ++] = s_mdbs_rcv.pqbuf[s_mdbs_rcv.head ++];
        if (s_mdbs_rcv.head == s_mdbs_rcv.size)
        {
            s_mdbs_rcv.head = 0;
        }
        if (len == s_mdbs_parse.bufsize) //Error, parse buffer overflow
        {
            len = 0;
        }
    }
    
    //If data formate is ASCII convert to common formate
    if (s_compara.ModbusType == MODBUS_ASCII)
    {
        //Conver ASCII formate to common formate
        do{
            if (len < 3)
                break;
            
            ptmp = s_mdbs_parse.pbuf;
            if (*ptmp != ASCII_FRAME_HEADER)
                break;
            
            ptmp = s_mdbs_parse.pbuf + len - 2;
            if (*ptmp != ASCII_FRAME_ENTER)
                break;
            
            ptmp ++;
            if (*ptmp != ASCII_FRAME_NEWLINE)
                break;
            
            if (!COMMON_ConverAscii2CommonMode(s_mdbs_parse.pbuf, len))
                break;
            
            ptmp = s_mdbs_parse.pbuf;
            len = (len - 5) / 2;    //Remove 0x3A 0x0D 0x0A and LRC
            
            if ((*ptmp) && (*ptmp) != s_compara.SlaveAddr)
                break;
            
            len --; //Remove slave address
            if (*ptmp ++)
            {
                s_mdbs_parse.pparse( ptmp, len, 0 );
            }
            else
            {
                s_mdbs_parse.pparse( ptmp, len, 1 );
            }
        }while(0);
    }
    else
    {
        //Check CRC and strip address
        do{
            if (len < 4)
                break;
            
            //Judge slaver address
            ptmp = s_mdbs_parse.pbuf;
            if ((*ptmp) && (*ptmp) != s_compara.SlaveAddr)
                break;
            
            len -= 2;   //CRC length 2 bytes:CRC_L, CRC_H
            if (!COMMON_GeneratorU16bitsCRCCheckCode( &crc_chk, s_mdbs_parse.pbuf, len ))
                break;
            
            ptmp += len;
            tmp = *ptmp ++;
            tmp |= (*ptmp) << 8;
            
            if (tmp != 0xFFFF)
            {
                if (tmp != crc_chk) //CRC check error
                    break;
            }
            len --;
            ptmp = s_mdbs_parse.pbuf;
            if (*ptmp ++)
            {
                s_mdbs_parse.pparse( ptmp, len, 0 );
            }
            else
            {
                s_mdbs_parse.pparse( ptmp, len, 1 );
            }
        }
        while (0);
    }

    return false;
}


/***************************************************************************************
Name:   
Func:   
Para:   
Retn:  
***************************************************************************************/
void USART1_IRQHandler( void )
{
    //Usart1 idle interrupt
    if (USART_GetITStatus( MODBUS_USARTNUM, USART_IT_IDLE ))
	{
        USART_ClearITPendingBit( MODBUS_USARTNUM, USART_IT_IDLE );
        
        if (modbus_idle_int_flg)
        {
            modbus_idle_int_flg = 0;
        }
        else
        {
            u32 dlyms;
            dlyms = (s_compara.ModbusType == MODBUS_ASCII) ? MODBUS_ASCII_DLY : MODBUS_RTU_DLY;
            DMACurDataCounter = DMA_GetCurrDataCounter( MODBUSRXD_DMA_CHANNEL );
            STM32FX_Systick_TimerSet( eTmr_ModbusTimer, STM32FX_Modbus_ModbusTimer, NULL, dlyms );
        }
	}
}



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:  
***************************************************************************************/
void DMA1_Channel2_3_IRQHandler( void )
{
     if (DMA_GetITStatus(DMA1_FLAG_TC2) || DMA_GetITStatus(DMA1_FLAG_TE2))
    {
        //Clear interrupt flag
        DMA_Cmd( MODBUSTXD_DMA_CHANNEL, DISABLE );
        DMA_ClearFlag( MODBUSTXD_DMA_INTFLAG );
        TIM_ITConfig( TIM7, TIM_IT_Update, ENABLE );
        TIM_Cmd( TIM7,ENABLE );
    }
}



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:  
***************************************************************************************/
static void STM32FX_Modbus_Timer7Config( void )
{
    TIM_TimeBaseInitTypeDef s_timer; 
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7,ENABLE);
    
	s_timer.TIM_Prescaler = 47999;     //	1MHz 1us
  	s_timer.TIM_Period = 10;     //	10000us
  	s_timer.TIM_CounterMode = TIM_CounterMode_Up;
  	s_timer.TIM_ClockDivision = TIM_CKD_DIV1;
  	TIM_TimeBaseInit(TIM7, &s_timer);
  
  	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPriority = 0x06;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);

  	TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
  	TIM_ITConfig(TIM7, TIM_IT_Update, DISABLE);
  	TIM_Cmd(TIM7,DISABLE);
  	TIM_SetCounter(TIM7, 0);
}



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:  
***************************************************************************************/
void TIM7_IRQHandler( void )
{
    if(TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)
	{
        TIM_ClearITPendingBit(TIM7, TIM_IT_Update);

        if (u_flag.flgbits.flgtransing)
        {
            u_flag.flgbits.flgtransing = 0;
            RS485_RX_ENABLE();
        }
        
        TIM_Cmd(TIM7,DISABLE);
        TIM_SetCounter(TIM7, 0);
        TIM_ITConfig(TIM7, TIM_IT_Update, DISABLE);
	}
}



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:  
***************************************************************************************/
bool STM32FX_Modbus_CommunicateInit( PS_COMM_BUFPARA pbufpara, ModbusParseFunc pFunc )
{
    bool ret;
    
    //Parameter init
    memset( &s_mdbs_rcv, 0, sizeof(S_QUEUE_TAG));   //Receive RS485 data queue
    memset( &s_mdbs_snd, 0, sizeof(S_SNDDAT_TAG));  //Send data to RS485 parameter init
    memset( &s_mdbs_parse, 0, sizeof(S_PARSE_TAG)); //Paser data init
    
    //Receive, send and paser parameter init
    do{
        if ((!pbufpara->prcv) || (!pbufpara->psnd) || (!pbufpara->pparse))
        {
            ret = false;
            break;
        }
        
        if ((!pbufpara->rcvsize) || (!pbufpara->sndsize) || (!pbufpara->parsesize))
        {
            ret = false;
            break;
        }
        
        s_mdbs_rcv.pqbuf = pbufpara->prcv;
        s_mdbs_rcv.size = pbufpara->rcvsize;
        
        s_mdbs_snd.psndbuf = pbufpara->psnd;
        s_mdbs_snd.bufsize = pbufpara->sndsize;
        
        s_mdbs_parse.pbuf = pbufpara->pparse;
        s_mdbs_parse.bufsize = pbufpara->parsesize;
        s_mdbs_parse.pparse = pFunc;
    }while (0);

//    //Timer3 congfig
    STM32FX_Modbus_Timer7Config( );
                        
    //USART1 port and pin init
    STM32FX_Modbus_CommunicatePortPinAndIntInit();
    
    //USART receive DMA init
    STM32FX_Modbus_CommunicateDMAConfig( );
    
    STM32FX_Modbus_CommunicateCommParaConfig( &s_compara );
    return ret;
}



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
static void STM32FX_Modbus_CommunicatePortPinAndIntInit( void )
{
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    //Init USART1 NVIC
    NVIC_InitStructure.NVIC_IRQChannel = MODBUS_USARTIRQN;
    NVIC_InitStructure.NVIC_IRQChannelPriority = MODBUS_USARTPRIO;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    //Init DMA1_TxD NVIC
    NVIC_InitStructure.NVIC_IRQChannel = MODBUSTXD_DMA_IRQN;
    NVIC_InitStructure.NVIC_IRQChannelPriority = MODBUSTXD_DMA_PRIO;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    //Enable clock
    RCC_AHBPeriphClockCmd(MODBUS_PORT_RCC, ENABLE);
    RCC_APB2PeriphClockCmd(MODBUS_USART_RCC, ENABLE);
    
    //Communicate port and pin inite
    GPIO_PinAFConfig(MODBUS_AFSOURCEPORT, MODBUS_TXD_AFSOURCE, GPIO_AF_1);
    GPIO_PinAFConfig(MODBUS_AFSOURCEPORT, MODBUS_RXD_AFSOURCE, GPIO_AF_1);
    
    GPIO_InitStructure.GPIO_Pin = MODBUS_RXD_PIN | MODBUS_TXD_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init( MODBUS_TXD_PORT, &GPIO_InitStructure );
    
    //RS485 control pin init
    RCC_AHBPeriphClockCmd(RS485_RCC_SOURCE, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = RS485_RX_CTRL_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init( RS485_RX_CTRL_PORT, &GPIO_InitStructure );
    RS485_RX_ENABLE();
}



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
void STM32FX_Modbus_CommunicateCommParaConfig( PS_COMM_PARA pcompara )
{
    USART_InitTypeDef USART_InitStru;
    
    memcpy( (u8*)&s_compara, (u8*)pcompara, sizeof(S_COMM_PARA) );
    
    //Communicate USART init
	switch (pcompara->BaudRate)
    {
    case 1:
        USART_InitStru.USART_BaudRate = 2400;
        break;
        
    case 2:
        USART_InitStru.USART_BaudRate = 4800;
        break;
        
    case 3:
        USART_InitStru.USART_BaudRate = 9600;
        break;
        
    case 4:
        USART_InitStru.USART_BaudRate = 19200;
        break;
        
    case 5:
        USART_InitStru.USART_BaudRate = 38400;
        break;
        
    case 6:
        USART_InitStru.USART_BaudRate = 57600;
        break;
        
    case 7:
        USART_InitStru.USART_BaudRate = 115200;
        break;
        
    default:
        USART_InitStru.USART_BaudRate = 9600;
        break;
    }
    
    switch (pcompara->Parity)
    {
        case 1:
            USART_InitStru.USART_Parity = USART_Parity_Odd;
            USART_InitStru.USART_StopBits = USART_StopBits_1;
            USART_InitStru.USART_WordLength = USART_WordLength_9b;
            break;
            
        case 2:
            USART_InitStru.USART_Parity = USART_Parity_Even;
            USART_InitStru.USART_StopBits = USART_StopBits_1;
            USART_InitStru.USART_WordLength = USART_WordLength_9b;
            break;
            
        case 3:
            USART_InitStru.USART_Parity = USART_Parity_No;
            USART_InitStru.USART_StopBits = USART_StopBits_2;
            USART_InitStru.USART_WordLength = USART_WordLength_8b;
            break;
            
        default:
            USART_InitStru.USART_Parity = USART_Parity_No;
            USART_InitStru.USART_StopBits = USART_StopBits_2;
            USART_InitStru.USART_WordLength = USART_WordLength_8b;
            break;
    }
    
	USART_InitStru.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStru.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init( MODBUS_USARTNUM, &USART_InitStru );
    
    USART_ClearITPendingBit( MODBUS_USARTNUM, USART_IT_IDLE );
    USART_ClearITPendingBit( MODBUS_USARTNUM, USART_IT_IDLE );
	USART_ITConfig( MODBUS_USARTNUM, USART_IT_IDLE, ENABLE );
    USART_ITConfig( MODBUS_USARTNUM, USART_IT_TXE, DISABLE );
	USART_ITConfig( MODBUS_USARTNUM, USART_IT_RXNE, DISABLE );
	USART_ClearFlag( MODBUS_USARTNUM, USART_FLAG_TC );
    USART_ClearITPendingBit( MODBUS_USARTNUM, USART_IT_IDLE );
    USART_Cmd( MODBUS_USARTNUM, ENABLE );
}

/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
static void STM32FX_Modbus_CommunicateDMAConfig( void )
{
    DMA_InitTypeDef DMA_InitStructure;

	RCC_AHBPeriphClockCmd( RCC_AHBPeriph_DMA1, ENABLE );
    
    //Rx DMA Channel init
	DMA_DeInit( MODBUSRXD_DMA_CHANNEL );
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&(MODBUS_USARTNUM->RDR));
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)s_mdbs_rcv.pqbuf;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = s_mdbs_rcv.size;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init( MODBUSRXD_DMA_CHANNEL, &DMA_InitStructure );

	USART_DMACmd( MODBUS_USARTNUM, USART_DMAReq_Rx, ENABLE );
	DMA_Cmd( MODBUSRXD_DMA_CHANNEL, ENABLE );
    
    //Tx DMA Channel init
    //DMA_DeInit( MODBUSTXD_DMA_CHANNEL );
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&(MODBUS_USARTNUM->TDR));
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)s_mdbs_snd.psndbuf;
    DMA_InitStructure.DMA_BufferSize = s_mdbs_snd.bufsize;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init( MODBUSTXD_DMA_CHANNEL, &DMA_InitStructure );
    
    DMA_ClearFlag( MODBUSTXD_DMA_INTFLAG );
    DMA_ITConfig( MODBUSTXD_DMA_CHANNEL, DMA_IT_TC | DMA_IT_TE, ENABLE);
    USART_DMACmd( MODBUS_USARTNUM, USART_DMAReq_Tx, ENABLE );
}


/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
static void STM32FX_Modbus_CommunicateResponseInformation( u16 datlen )
{
    u8* pans;
    
    if (!datlen)
        return;
    
    //Response operator result
    if (s_compara.ModbusType == MODBUS_ASCII)
    {
        u8 lrcchk;
        lrcchk = COMMON_GeneratorLRCCheckCode( s_mdbs_snd.psndbuf, datlen);
        pans = s_mdbs_snd.psndbuf + datlen;
        *pans = lrcchk;
        datlen ++;
        if (!COMMON_ConvertHex2Ascii( s_mdbs_snd.psndbuf, datlen ))
            return;
        datlen *= 2;
        pans = s_mdbs_snd.psndbuf;
        *pans++ = ASCII_FRAME_HEADER;
        pans += datlen;
        COMMON_Bits16Convert2Bits8( pans, ASCII_FRAME_ENDFLAG, BIGENDDIAN );
        datlen += 3;
    }
    else
    {
        u16 crc_chk;
        COMMON_GeneratorU16bitsCRCCheckCode( &crc_chk, s_mdbs_snd.psndbuf, datlen );
        COMMON_Bits16Convert2Bits8( s_mdbs_snd.psndbuf + datlen, crc_chk, LITTLEENDIAN );
        datlen += 2;
    }
    
    //Send response imformation
    STM32FX_Modbus_CommunicateSendData( datlen );
}


/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
#define UPGRADE_SUCCEED 0x00
#define UPGRADE_RCVFAIL 0x01
#define UPGRADE_CHKERR  0x02
void STM32FX_Modbus_UpgradeResponseProcess( u8* psrc, u16 len )
{
    u8* ptmp;
    
    ptmp = s_mdbs_snd.psndbuf;
    *ptmp ++ = s_compara.SlaveAddr;
    memcpy(ptmp, psrc, 3);
    ptmp += 3;
    COMMON_Bits16Convert2Bits8( ptmp, len, BIGENDDIAN );
    ptmp += 2;
    memcpy(ptmp, (psrc + 3), len);

    STM32FX_Modbus_CommunicateResponseInformation( len + 6 );
}



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
void STM32FX_Modbus_CommunicateSendData( u16 len )
{
    if ((!s_mdbs_snd.psndbuf) || (!len) || (len >= s_mdbs_snd.bufsize))
        return;
    
    //Enalbe trans data
    RS485_TX_ENABLE();
    u_flag.flgbits.flgtransing = 1;
    MODBUSTXD_DMA_CHANNEL->CNDTR = len;
    DMA_Cmd( MODBUSTXD_DMA_CHANNEL, ENABLE );
}



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
bool STM32FX_Modbus_ModbusIsSendingData( void )
{
    if (u_flag.flgbits.flgtransing)
        return true;
    else
        return false;
}


//================================================================================================
/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
bool STM32FX_Modbus_FunctionCode03ParameterInit( PS_F03_EX_PARA pdat )
{
    bool ret = true;
    
    do{
        if ((!pdat->pAddrTab) || (!pdat->pStVal))
        {
            ret = false;
            break;
        }
        memset( &s_f03, 0, sizeof(S_F03_EX_PARA) );
        s_f03.pStVal = pdat->pStVal;
        s_f03.pAddrTab = pdat->pAddrTab;
        s_f03.paranum = pdat->paranum;
    }while (0);
    
    return ret;
}



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
bool STM32FX_Modbus_FunctionCode04ParameterInit( PS_F04_EX_PARA pdat )
{
    bool ret = true;
    
    do{
        if ((!pdat->pAddrTab) || (!pdat->pStVal))
        {
            ret = false;
            break;
        }
        memset( &s_f04, 0, sizeof(S_F04_EX_PARA) );
        s_f04.pStVal = pdat->pStVal;
        s_f04.pAddrTab = pdat->pAddrTab;
        s_f04.paranum = pdat->paranum;
    }while(0);
    
    return ret;
}


/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
bool STM32FX_Modbus_FunctionCode26ParameterInit( PS_F26_EX_PARA pdat )
{
    bool ret = true;
    
    do{
        if ((!pdat->pAddrTab) || (!pdat->pStVal))
        {
            ret = false;
            break;
        }
        memset( &s_f26, 0, sizeof(S_F26_EX_PARA) );
        s_f26.pStVal = pdat->pStVal;
        s_f26.pAddrTab = pdat->pAddrTab;
        s_f26.paranum = pdat->paranum;
    }while(0);
    
    return ret;
}



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
bool STM32FX_Modbus_FunctionCode05ParameterInit( PS_F05_EX_PARA pdat )
{
    bool ret = true;
    
    do{
        if ((!pdat->pAddrTab) || (!pdat->pStVal))
        {
            ret = false;
            break;
        }
        memset( &s_f05, 0, sizeof(S_F05_EX_PARA) );
        s_f05.pStVal = pdat->pStVal;
        s_f05.pAddrTab = pdat->pAddrTab;
        s_f05.paranum = pdat->paranum;
    }while(0);
    
    return ret;
}



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
bool STM32FX_Modbus_FunctionCode25ParameterInit( PS_F25_EX_PARA pdat )
{
    bool ret = true;
    
    do{
        if ((!pdat->pAddrTab) || (!pdat->pStVal))
        {
            ret = false;
            break;
        }
        memset( &s_f25, 0, sizeof(S_F25_EX_PARA) );
        s_f25.pStVal = pdat->pStVal;
        s_f25.pAddrTab = pdat->pAddrTab;
        s_f25.paranum = pdat->paranum;
    }while(0);
    
    return ret;
}


/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
bool STM32FX_Modbus_FunctionCode2AParameterInit( PS_F2A_EX_PARA pdat )
{
    bool ret = true;
    
    do{
        if ((!pdat->pAddrTab) || (!pdat->pString))
        {
            ret = false;
            break;
        }
        memset( &s_f2A, 0, sizeof(S_F2A_EX_PARA) );
        s_f2A.pString = pdat->pString;
        s_f2A.pAddrTab = pdat->pAddrTab;
        s_f2A.paranum = pdat->paranum;
    }while(0);
    
    return ret;
}


/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
#define BEIJING_JIKANG_ADDR 0xFF00
Modbus_Result STM32FX_Modbus_FunctionCodeReadData( u8* psrc )
{
    u8  sendlen;
    u8* ptmp;
    u16 regnum;
    u16 randval;
    Modbus_Result re = ModbusOperatorSucceed;
    
    ptmp = psrc + 1;    //f_cmd, star_addr_l, star_addr_h, reg_num_l, reg_num_h, ...
    GET_BIGENDIAN16(randval, ptmp);
    GET_BIGENDIAN16(regnum, ptmp);
    
    switch (*psrc)
    {
        case FUNC03_READ:
//            re = STM32FX_Modbus_FunctionCode03Process( regnum, randval );
             if (randval)                       //查设备地址命令，消息ID为FF 00
            {
                if (randval == BEIJING_JIKANG_ADDR) //Only for BeijingJK
                    randval = 0x0030;
                
                re = STM32FX_Modbus_FunctionCode03Process( regnum, randval );
            }
            else                                //读数据命令，消息ID为00 00
            {
                randval = 0x0006;   //Use position3 address, need to modify UserApp func MAGNETAPP_ProcessSensorData
                re = STM32FX_Modbus_FunctionCode04Process( regnum, randval );
            }
            break;
            
        case FUNC04_READ:
            re = STM32FX_Modbus_FunctionCode04Process( regnum, randval );
            break;
            
        case FUNC26_READ:
            re = STM32FX_Modbus_FunctionCode26Process( regnum, randval );
            break;
            
        case FUNC2B_READ:
            re = STM32FX_Modbus_FunctionCode2BProcess( regnum, &randval );
            break;
    }
    
    ptmp = s_mdbs_snd.psndbuf;
    *ptmp++ = s_compara.SlaveAddr;
    if (re == ModbusOperatorSucceed)
    {
        *ptmp ++ = *psrc;
        if (*psrc != FUNC2B_READ)
        {
            *ptmp = regnum * 2;
            sendlen = regnum * 2 + 3;
        }
        else
        {
            *ptmp ++ = regnum >> 8;
            *ptmp = (u8)regnum;
            sendlen = randval + 4;
        }
    }
    else
    {
        *ptmp ++ = *psrc | MODBUS_CMD_ERR_MASK;
        *ptmp = re;
        sendlen = 3;
    }
    
    //Send response imformation
    STM32FX_Modbus_CommunicateResponseInformation( sendlen );
    
    return re;
}


/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
static Modbus_Result STM32FX_Modbus_FunctionCode03Process( u16 reg_num, u16 star_addr )
{
    u8 i;
    u8* pdest;
    
    Modbus_Result ret = ModbusOperatorSucceed;
    
    do{
        if ((!reg_num) || (reg_num > s_f03.paranum))
        {
            ret = RegisterNumberError;
            break;
        }
        for (i = 0; i < s_f03.paranum; i ++)
        {
            if (s_f03.pAddrTab[i] == star_addr)
            {
                break;
            }
        }
        if (i >= s_f03.paranum)
        {
            ret = ReadDatItemNotSupport;
            break;
        }
        
        pdest = s_mdbs_snd.psndbuf + 3;  //slv_addr f_cmd, dat_len, {data_list}
        while (reg_num --)
        {
            COMMON_Bits16Convert2Bits8( pdest, *(s_f03.pStVal + i), BIGENDDIAN);
            pdest += 2;
            if (++ i > s_f03.paranum)
            {
                ret = OperatorPraraError;
                break;
            }
        }
    }while (0);
    
    return ret;
}


/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
static Modbus_Result STM32FX_Modbus_FunctionCode04Process( u16 reg_num, u16 star_addr )
{
    u8 i;
    u8* pdest;
    
    Modbus_Result ret = ModbusOperatorSucceed;
    
    do{
        if ((reg_num < 2) || (reg_num % 2) || (reg_num / 2 > s_f04.paranum))
        {
            ret = RegisterNumberError;
            break;
        }
        for (i = 0; i < s_f04.paranum; i ++)
        {
            if (s_f04.pAddrTab[i] == star_addr)
            {
                break;
            }
        }
        if (i >= s_f04.paranum)
        {
            ret = ReadDatItemNotSupport;
            break;
        }
        
        pdest = s_mdbs_snd.psndbuf + 3;  //slv_addr f_cmd, dat_len, {data_list}
        while (reg_num)
        {
            COMMON_Bits32Convert2Bits8( pdest, *(s_f04.pStVal + i), BIGENDDIAN);
            pdest += 4;
            reg_num -= 2;
            if (++ i > s_f04.paranum)
            {
                ret = OperatorPraraError;
                break;
            }
        }
    }while (0);
    
    return ret;
}



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
static Modbus_Result STM32FX_Modbus_FunctionCode26Process( u16 reg_num, u16 star_addr )
{
    u8 i;
    u8* pdest;
    
    Modbus_Result ret = ModbusOperatorSucceed;
    do{
        if ((reg_num < 2) || (reg_num % 2) || (reg_num / 2 > s_f26.paranum))
        {
            ret = RegisterNumberError;
            break;
        }
        for (i = 0; i < s_f26.paranum; i ++)
        {
            if (s_f26.pAddrTab[i] == star_addr)
            {
                break;
            }
        }
        if (i >= s_f26.paranum)
        {
            ret = ReadDatItemNotSupport;
            break;
        }
        
        pdest = s_mdbs_snd.psndbuf + 3;  //slv_addr f_cmd, dat_len, {data_list}
        while (reg_num)
        {
            COMMON_Bits32Convert2Bits8( pdest, *(s_f26.pStVal + i), BIGENDDIAN);
            pdest += 4;
            reg_num -= 2;
            if (++ i > s_f26.paranum)
            {
                ret = OperatorPraraError;
                break;
            }
        }
    }while (0);
    
    return ret;
}



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
static Modbus_Result STM32FX_Modbus_FunctionCode10Process( u8* psrc, u16 reg_num, u16 star_addr )
{
    u8 i;
    //datlen {datalist}
    Modbus_Result ret = ModbusOperatorSucceed;
    
    do{
        if (!psrc)
        {
            ret = FuncOperationFailed;
            break;
        }
        if ((!reg_num) || (reg_num > s_f03.paranum) || (reg_num * 2 != *psrc))
        {
            ret = RegisterNumberError;
            break;
        }
        for (i = 0; i < s_f03.paranum; i ++)
        {
            if (s_f03.pAddrTab[i] == star_addr)
            {
                break;
            }
        }
        if (i >= s_f03.paranum)
        {
            ret = ReadDatItemNotSupport;
            break;
        }
        
        psrc ++;
        while (reg_num --)
        {
            COMMON_Bits8Convert2Bits16( s_f03.pStVal + i, psrc, BIGENDDIAN );
            psrc += 2;
            if (++ i > s_f03.paranum)
            {
                ret = OperatorPraraError;
                break;
            }
        }
    }while (0);
    
    return ret;
}



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
static Modbus_Result STM32FX_Modbus_FunctionCode27Process( u8* psrc, u16 reg_num, u16 star_addr )
{
    u8 i;
    //datlen {datalist}
    Modbus_Result ret = ModbusOperatorSucceed;
    
    do{
        if (!psrc)
        {
            ret = FuncOperationFailed;
            break;
        }
        if ((reg_num < 2) || (reg_num / 2 > s_f26.paranum) || (reg_num * 2 != *psrc))
        {
            ret = RegisterNumberError;
            break;
        }
        for (i = 0; i < s_f26.paranum; i ++)
        {
            if (s_f26.pAddrTab[i] == star_addr)
            {
                break;
            }
        }
        if (i >= s_f26.paranum)
        {
            ret = ReadDatItemNotSupport;
            break;
        }
        
        psrc ++;
        while (reg_num)
        {
            COMMON_Bits8Convert2Bits32( s_f26.pStVal + i, psrc, BIGENDDIAN );
            psrc += 4;
            reg_num -= 2;
            if (++ i > s_f26.paranum)
            {
                ret = OperatorPraraError;
                break;
            }
        }
    }while (0);
    
    return ret;
}


/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
static Modbus_Result STM32FX_Modbus_FunctionCode2BProcess( u16 reg_num, u16* star_addr )
{
    u8 i;
    u8* pdest;
    u8* prec;
    u16 len;
    u16 totlen = 0;
    const char* ptmp;
    Modbus_Result ret = ModbusOperatorSucceed;
    
    (*star_addr) &= 0xFF;
    do{
        if (reg_num > s_f2A.paranum)
        {
            ret = RegisterNumberError;
            break;
        }
        for (i = 0; i < s_f2A.paranum; i ++)
        {
            if (s_f2A.pAddrTab[i] == (*star_addr))
            {
                break;
            }
        }
        if (i >= s_f2A.paranum)
        {
            ret = ReadDatItemNotSupport;
            break;
        }

        pdest = s_mdbs_snd.psndbuf + 4;  //slv_addr f_cmd 00 reg_num_h item1_id item1_len item1_data
        while (reg_num --)
        {
            len = 0;
            *pdest ++ = (u8)(s_f2A.pAddrTab[i]);
            prec = pdest ++;
            ptmp = s_f2A.pString + i * FUNC2ALEN;
            while (*ptmp && len < FUNC2ALEN)
            {
                *pdest ++ = *ptmp ++;
                len ++;
            }
            
            if (len >= FUNC2ALEN)
            {
                ret = OperatorPraraError;
                break;
            }
            *prec = len;
            totlen += len + 2;
            if (++ i > s_f2A.paranum)
            {
                ret = OperatorPraraError;
                break;
            }
        }
        *star_addr = totlen;
    }while (0);
    
    return ret;
}


/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
    f_code star_addr_h star_addr_l FF(00) 00 CRC_L CRC_H
***************************************************************************************/
Modbus_Result STM32FX_Modbus_FunctionCode05_25WriteData( u8* psrc )
{
    u8 i;
    u8* ptmp;
    u16 star_addr;
    u16 logic_val;
    
    Modbus_Result re = ModbusOperatorSucceed;
    do{
        if (!psrc)
        {
            re = FuncOperationFailed;
            break;  //break do{}while(0)
        }
        ptmp = psrc + 1;
        GET_BIGENDIAN16(star_addr, ptmp);
        GET_BIGENDIAN16(logic_val, ptmp);
        switch (*psrc)
        {
        case FUNC05_WRITE:
            for (i = 0; i < s_f05.paranum; i ++)
            {
                if (s_f05.pAddrTab[i] == star_addr)
                    break;  //break for
            }
            if (i >= s_f05.paranum)
            {
                re = ReadDatItemNotSupport;
                break;  //break switch
            }
            if (logic_val == MODBUS_LOGIC_TRUE)
            {
                *(s_f05.pStVal + i) = SOWAY_LOGIC_TRUE;
            }
            else if (logic_val == MODBUS_LOGIC_FALSE)
            {
                *(s_f05.pStVal + i) = SOWAY_LOGIC_FALSE;
            }
            else
                re = FuncOperationFailed;
            
            break;  //break switch
            
        case FUNC25_WRITE:
            for (i = 0; i < s_f25.paranum; i ++)
            {
                if (s_f25.pAddrTab[i] == star_addr)
                    break;  //break for
            }
            if (i >= s_f25.paranum)
            {
                re = ReadDatItemNotSupport;
                break;  //break switch
            }
            if (logic_val == MODBUS_LOGIC_TRUE)
            {
                *(s_f25.pStVal + i) = SOWAY_LOGIC_TRUE;
            }
            else if (logic_val == MODBUS_LOGIC_FALSE)
            {
                *(s_f25.pStVal + i) = SOWAY_LOGIC_FALSE;
            }
            else
                re = FuncOperationFailed;
            
            break;  //break switch
        }
    }while (0);
    return re;
}


/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
void STM32FX_Modbus_WriteParameterFunctionResponse( u8* psrc, Modbus_Result res )
{
    u8* ptmp;
    u16 sndlen;
    
    ptmp = s_mdbs_snd.psndbuf;
    *ptmp ++ = s_compara.SlaveAddr;
    
    if (res == ModbusOperatorSucceed)
    {
        memcpy( ptmp, psrc, 5); //slv_addr, f_cmd, star_addr_h, star_addr_l, reg_num_h, reg_num_l
        sndlen = 6; //default operator succeed
    }
    else
    {
        *ptmp ++ = *psrc | MODBUS_CMD_ERR_MASK;
        *ptmp ++ = res;
        sndlen = 3;
    }
    STM32FX_Modbus_CommunicateResponseInformation( sndlen );
}


/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
Modbus_Result STM32FX_Modbus_FunctionCode10_27WriteData( u8* psrc )
{
    u8* ptmp;
    u16 reg_num;
    u16 star_addr;
    //f_cmd, star_addr_l, star_addr_h, reg_num_l, reg_num_h, reg_num_l, datlen, {datalist}
    
    Modbus_Result re = ModbusOperatorSucceed;
    
    do{
        if (!psrc)
        {
            re = FuncOperationFailed;
            break;
        }
        ptmp = psrc + 1;
        GET_BIGENDIAN16(star_addr, ptmp);
        GET_BIGENDIAN16(reg_num, ptmp);
        switch (*psrc)
        {
        case FUNC10_WRITE:
            if (star_addr == BEIJING_JIKANG_ADDR)   //Only for BeijingJK
                star_addr = 0x0030;
            
            re = STM32FX_Modbus_FunctionCode10Process( ptmp, reg_num, star_addr );
            break;
            
        case FUNC27_WRITE:
            re = STM32FX_Modbus_FunctionCode27Process( ptmp, reg_num, star_addr );
            break;
        }
    }while (0);

    return re;
}


/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
Modbus_Result STM32FX_Modbus_FunctionCode2AWriteInformation( u8* psrc )
{
    u8 i, len;
    u8* pdest;
    u8* ppara;
    u16 obn, infid;
    Modbus_Result ret = ModbusOperatorSucceed;

    ppara = psrc + 5;
    psrc += 1;
    GET_BIGENDIAN16( infid, psrc );
    infid &= 0xFF;
    GET_BIGENDIAN16( obn, psrc );
    do{
        for (i = 0; i < s_f2A.paranum; i ++)
        {
            if (s_f2A.pAddrTab[i] == infid)
                break;
        }
        if (i + obn > s_f2A.paranum)
        {
            ret = ReadDatItemNotSupport;
            break;
        }
        
        pdest = s_mdbs_snd.psndbuf;
        memset( pdest, 0, s_mdbs_snd.bufsize );
        *pdest ++ = i;          //offset value
        *pdest ++ = (u8)obn;    //object 
        
        while (obn --)
        {
            len = *psrc ++;
            memcpy( pdest, psrc, len );
            psrc += len;
            pdest += FUNC2ALEN;
        }
        pdest = s_mdbs_snd.psndbuf + 1;
        memcpy( ppara, s_mdbs_snd.psndbuf, (*pdest) * FUNC2ALEN + 2);
    }while (0);
    
    return ret;
}
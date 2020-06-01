//This is the Modbus test C source file
/***************************************************************************************
                    This file is the Modbus C source file

                Edited by xie_yuyin 2019-05-06 for stm32f072 second edition
***************************************************************************************/
//Include files
#include "STM32FX_Systick.h"
#include "stm32fxmodbuslib.h"
#include "Display.h"
#include "TDC_GP21.h"
#include "SPI1.h"
#include "Analog.h"

#include "Flashop.h"
#include "UserApp.h"
#include "AppDatabase.h"

//**************************************************************************************
//Define parameters
const u8 presetsyspara[] = {
    0xAA,       //0xAA:Chip init flag; 
    0x00,       //0xFF:Upgrade flag
    0x01,       //salve address
    0x03,       //default baud 9600
    0x03,       //No parity
    0x01,       //Modbus data type 1--RTU   2--ASCII
    0x00, 0x03, //Filter number
    0x14,       //TDC update time: 5-0.5ms 10-1ms   20-2ms
    0x00,       //Auto upload time
    0x01,       //Output unit
    0x00,       //Modbus type 0:Modbus RTU 7:Modbus ASCII 1,2,3,4,5,6 corresponding AD5422 output mode
    0x00,       //Positive or reverse 0:Positive 1:reverse
    
    0x00, 0x00, //Position alarm threshold 1 
    0x00, 0x00, //return difference 1
    0xFF, 0xFF, //Position alarm threshold 2
    0x00, 0x00, //return difference 2
    
    0x01,   //Magnet rings number
    0x00,   //Enable linear compare 0:disable 1:enable
    0x00,   //Enable OLED display   0:disable 1:enable
    
    0x00, 0x01, //Temprature compensate K
    0x00, 0x00, //Temprature compensate B
    0x01, 0xF4, //PWC value
    
    0x01, 0x2C, 0x00, 0x00, //Range 300mm
    0x00, 0x54, 0x71, 0x76, //Zero point time value
    0x01, 0x94, 0xAA, 0x01, //Full point time value
};

//Define string
const char DeviceInformation[COMPANY_INFO_ITEMNUM][FUNC2ALEN] = {
    "738830676",
    "Product ID",
    "Soway Magnet HW_Ver: PCBM001(C)",
    "Soway Magnet SW_Ver: V0.03.00_200214",
    "Device ID",
    "Customer ID",
    "",
};

//Variables define
//This part define in user App.c
//Modbus receive buffer size
#define MODBUS_RCVBUFSIZE      1024
static u8 mdbus_rcvbuf[MODBUS_RCVBUFSIZE];

//Send data parameter
#define PC_COMM_SNDBUFSIZE      512
static u8 sndbuf[PC_COMM_SNDBUFSIZE];

//Parse data parameter
#define MSG_PARSE_BUFSIZE    512
static u8 parsebuf[MSG_PARSE_BUFSIZE];
//This part define in user App.c end end end end end end

//define Application variables
typedef struct{
    u8  flgcalib;
    u8  flgcommp;
}S_REFRESH_TAG, *PS_REFRESH_TAG;

static S_REFRESH_TAG s_refflg;

//Modbus Func code variables
static S_F03CODE_PARA s_combine;    //f10
static S_F04CODE_PARA s_output_para;
static S_F26CODE_PARA s_snrval;     //f27
static S_F05CODE_PARA s_logic05;
static S_F25CODE_PARA s_logic25;

//Local functions declare
static void USERAPP_ModbusCommunicateParameteresInit( void );
static void USERAPP_GenerateSystemParameters( u8 );
static bool USERAPP_Implement05FuncCode( void );
static bool USERAPP_ImplementWriteParaFuncCode( u8 cmd );
static bool USERAPP_Implement2AFuncCode( u8* psrc );
static bool USERAPP_Implement25FuncCode( u8* psrc );
static bool USERAPP_WriteParameter2Flash( u8 wcmd, u8* p );
static void USERAPP_ADG779ControlTimerConfig( void );

static void USERAPP_ParseReceiveModbusData( u8* psrc, u32 rcvlen, u8 msgtype );

#define SYSPARA_BACKUP  1
#define SYSPARA_RESTORE 2

#define MAGNETAPP_CALIBRATION_OK    0x88
#define MAGNETAPP_CALIBRATION_ZERO  0x08
#define MAGNETAPP_CALIBRATION_FSS   0x80

//**************************************************************************************
//User variables define
static S_SENSOR_REF     s_snrref;   //1: normal 2:dead area 3:no magnet ring
static S_TDCDATA_TAG    s_tdcdat1;
static S_TDCDATA_TAG    s_tdcdat2;
static u32 u32_t_interval;
static unsigned long long u64_ad_facroty;

static u8 trig_flg = 0;
static u8 u8magnetnum = 1;
static u8 upgradeflg;

//Functions declare
static void TIM2_Configuration( void );
static void USERAPP_ExtInterruptConfig( void );
static void MAGNETAPP_ProcessSensorData( u32 tdc_avg, u8 index );



//Functions implement
/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
void USERApp_MagnetProcess( void )
{
    while (1)
    {
#ifdef USE_IDWDG
        FEED_I_WATCHDOG();
#endif
        STM32FX_Systick_TimerProcess( );
    }
}


/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
void USERApp_Init( void )
{
    //Static variables init
    memset(&s_snrref, 0, sizeof(S_SENSOR_REF));
    memset(&s_tdcdat1, 0, sizeof(S_TDCDATA_TAG));
    memset(&s_tdcdat2, 0, sizeof(S_TDCDATA_TAG));
    
    //Spi drive init
    SPI_Spi1DriveInite( );
    
    //AD5422 Channel1 inite
    ANALOG_ChipInit();
    
    //Modbus communicate parameteres init
    USERAPP_ModbusCommunicateParameteresInit();
    
    //Get system parameters from flash
    USERAPP_GenerateSystemParameters( 1 );
    
    //Display led init
    DISPLAY_LedInit( &s_snrref.deadareaflg );
    
    //TDC_GP21 Init
    TDC_TDCInit();
    
    //Ext interrupt init
    USERAPP_ExtInterruptConfig();
    
    //ADG779 Timer init
    USERAPP_ADG779ControlTimerConfig();
    
    //Timer2 configuration
    TIM2_Configuration();
}


/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
static void USERAPP_ModbusCommunicateParameteresInit( void )
{
    u16 i;
    
    //Communicate buffer parameters
    S_COMM_BUFPARA  s_comm_bufpara;
    memset( &s_comm_bufpara, 0, sizeof(S_COMM_BUFPARA) );
    s_comm_bufpara.prcv = mdbus_rcvbuf;
    s_comm_bufpara.rcvsize = MODBUS_RCVBUFSIZE;
    s_comm_bufpara.psnd = sndbuf;
    s_comm_bufpara.sndsize = PC_COMM_SNDBUFSIZE;
    s_comm_bufpara.pparse = parsebuf;
    s_comm_bufpara.parsesize = MSG_PARSE_BUFSIZE;
    STM32FX_Modbus_CommunicateInit( &s_comm_bufpara, USERAPP_ParseReceiveModbusData );
    
    memset( &s_refflg, 0, sizeof(S_REFRESH_TAG) );
    
    //Modbus function codes parameters
    //Functions code 03
    S_F03_EX_PARA s_exf03;
    memset( &s_combine, 0, sizeof(S_F03CODE_PARA) );
    i = sizeof(S_F03CODE_PARA) / FUNC03UNITS;
    if (i == sizeof(Func03CodeAddrTab) / COILADDRLEN)
    {
        s_exf03.pStVal = (u16*)&s_combine;
        s_exf03.pAddrTab = Func03CodeAddrTab;
        s_exf03.paranum = i;
        STM32FX_Modbus_FunctionCode03ParameterInit( &s_exf03 );
    }

    //Function code 04
    S_F04_EX_PARA s_exf04;
    memset( &s_output_para, 0, sizeof(S_F04CODE_PARA) );
    i = sizeof(S_F04CODE_PARA) / FUNC04UNITS;
    if (i == sizeof(Func04CodeAddrTab) / COILADDRLEN)
    {
        s_exf04.pStVal = (u32*)&s_output_para;
        s_exf04.pAddrTab = Func04CodeAddrTab;
        s_exf04.paranum = i;
        STM32FX_Modbus_FunctionCode04ParameterInit( &s_exf04 );
    }
    
    //Function code 26
    S_F26_EX_PARA s_exf26;
    memset( &s_snrval, 0, sizeof(S_F26_EX_PARA) );
    i = sizeof(S_F26CODE_PARA) / FUNC26UNITS;
    if (i == sizeof(Func26CodeAddrTab) / COILADDRLEN)
    {
        s_exf26.pStVal = (u32*)&s_snrval;
        s_exf26.pAddrTab = Func26CodeAddrTab;
        s_exf26.paranum = i;
        STM32FX_Modbus_FunctionCode26ParameterInit( &s_exf26 );
    }

    //F05 write logic parameter
    S_F05_EX_PARA s_exf05;
    memset( &s_logic05, 0, sizeof(S_F05CODE_PARA) );
    i = sizeof(S_F05CODE_PARA);
    if (i == sizeof(Func05CodeAddrTab) / COILADDRLEN)
    {
        s_exf05.pStVal = (u8*)&s_logic05;
        s_exf05.pAddrTab = Func05CodeAddrTab;
        s_exf05.paranum = i;
        STM32FX_Modbus_FunctionCode05ParameterInit( &s_exf05 );
    }
    
    //F25 write logic
    S_F25_EX_PARA s_exf25;
    memset( &s_logic25, 0, sizeof(S_F25CODE_PARA) );
    i = sizeof(S_F25CODE_PARA);
    if (i == sizeof(Func25CodeAddrTab) / COILADDRLEN)
    {
        s_exf25.pStVal = (u8*)&s_logic25;
        s_exf25.pAddrTab = Func25CodeAddrTab;
        s_exf25.paranum = i;
        STM32FX_Modbus_FunctionCode25ParameterInit( &s_exf25 );
    }
    
    //F2A/F2B
    S_F2A_EX_PARA s_exf2A;
    i = sizeof( Func2ACodeAddrTab ) / COILADDRLEN;
    if (i == F2A_2B_ITEM_NUM)
    {  
        s_exf2A.pString = (const char*)SOWAYFLASH_OP_BASE_ADDR;
        s_exf2A.pAddrTab = Func2ACodeAddrTab;
        s_exf2A.paranum = i;
        STM32FX_Modbus_FunctionCode2AParameterInit( &s_exf2A );
    }
}



/***************************************************************************************
Name:   
Func:   
Para:
Retn:
***************************************************************************************/
static void USERAPP_GenerateSystemParameters( u8 flg )
{
    u32 syspara[FLASH_PARA_DW_SIZE];    //Parameter flash size 1k, parameters 512, company info 512
    u8* pptr = (u8*)syspara;
    
    memset(pptr, 0, FLASH_PARA_DW_SIZE);
    FLASHOP_ReadDataFromChipFlash( pptr, SOWAYFLASH_OP_BASE_ADDR, FLASH_PARA_DW_SIZE, FLASH_PARA_DW_SIZE );
    pptr += USERPARA_INFO_OFFSET;
    if (*pptr != CHIP_INIT_FLAG)
    {
        pptr = (u8*)syspara;
        FLASHOP_ReadDataFromChipFlash( pptr, SOWAYFLASH_BKUPPARA_ADDR, FLASH_PARA_DW_SIZE, FLASH_PARA_DW_SIZE );
        pptr += USERPARA_INFO_OFFSET;
        if (*pptr != CHIP_INIT_FLAG)
        {//Init preset parameters
            pptr = (u8*)syspara;
            memcpy(pptr, (u8*)DeviceInformation, COMPANY_INFO_SIZE);
            pptr += USERPARA_INFO_OFFSET;
            memcpy(pptr, (u8*)presetsyspara, SYSPARA_NUM);
        }
        FLASHOP_EraseSpecifiedFlashPages( SOWAYFLASH_OP_BASE_ADDR, 1 );
        FLASHOP_WriteData2SpecifiedFlash( SOWAYFLASH_OP_BASE_ADDR, syspara, FLASH_PARA_DW_SIZE );
    }
    
    //End device ID
    s_combine.edv_addr = *(pptr + OFF_SLAVE_ID);
    s_combine.baud_rate = *(pptr + OFF_BAUDRATE);
    s_combine.oe_parity = *(pptr + OFF_PARITY);
    s_combine.modbusfmt = *(pptr + OFF_MODBUSFT);
    s_combine.analog_mode = *(pptr + OFF_ANALOGMODE);
    s_combine.direction_sel = *(pptr + OFF_DIRCTION);
    
     if (s_combine.analog_mode)
            ANALOG_AD5422OutPutParametersConfig( s_combine.analog_mode );
    
    u8magnetnum = *(pptr + OFF_MAGNETNUM);
    s_snrval.magnetnum = u8magnetnum << 16;
    
    //ADG779 timer value
    s_snrval.pwc_val = (*(pptr + OFF_PWCVALUE)) << 8;
    s_snrval.pwc_val |= *(pptr + OFF_PWCVALUE + 1);
    s_snrval.pwc_val <<= 16;

    COMMON_Bits8Convert2Bits32( &s_snrval.range, pptr + OFF_FSSRANGE, BIGENDDIAN );
    COMMON_Bits8Convert2Bits32( &s_snrval.zerotmv, pptr + OFF_ZEROPOINT, BIGENDDIAN );
    COMMON_Bits8Convert2Bits32( &s_snrval.fulltmv, pptr + OFF_FULLPOINT, BIGENDDIAN );
    
    u32_t_interval = s_snrval.fulltmv - s_snrval.zerotmv;
    u64_ad_facroty = ((unsigned long long)0xFFFF) << 32;
    u64_ad_facroty /= u32_t_interval;
    
    //Refresh communicate parameters, If needed
    if (flg)
    {
        S_COMM_PARA commpara;
        
        if (!s_combine.edv_addr)
        {
            s_combine.edv_addr = 1;
        }
        commpara.SlaveAddr = s_combine.edv_addr;
        
        if (s_combine.baud_rate > 7)
        {
            s_combine.baud_rate = 3;
        }
        commpara.BaudRate = s_combine.baud_rate;
        
        if (s_combine.oe_parity > 3)
        {
            s_combine.oe_parity = 3;
        }
        commpara.Parity = s_combine.oe_parity;
        
        if (s_combine.modbusfmt > 2) //1-RTU; 2-ASCII
        {
            s_combine.modbusfmt = 1;
        }
        commpara.ModbusType = s_combine.modbusfmt;
        
        while (STM32FX_Modbus_ModbusIsSendingData())
        {
#ifdef USE_IDWDG
            FEED_I_WATCHDOG();
#endif
        }
        STM32FX_Modbus_CommunicateCommParaConfig( &commpara );
    }
}




/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
f05 and f25 logic:  0x0F---False
                    0xF0---True
***************************************************************************************/
static void USERAPP_ParseReceiveModbusData( u8* psrc, u32 rcvlen, u8 msgtype )
{
    Modbus_Result ret;
    
    s_snrref.communicte = 1;
    
    if ((!psrc) || (!rcvlen))
        return;
    
    if ((msgtype) && ((*psrc) != FUNC03_READ))  //When slaver address is broadcast, only F03 received
    {
        return;
    }
    
    switch (*psrc)
    {
    case FUNC03_READ:
    case FUNC04_READ:
    case FUNC26_READ:
    case FUNC2B_READ:
         //Modified by XYY 20200403 for Beijin JiKang
        if (msgtype)
        {
            STM32FX_Systick_Delayxms( s_combine.edv_addr * 50 );
        }
        STM32FX_Modbus_FunctionCodeReadData( psrc );
        break;
        
    case FUNC05_WRITE:
    {
        ret = STM32FX_Modbus_FunctionCode05_25WriteData( psrc );
        if (ret == ModbusOperatorSucceed)
        {
            //Todo: Process 05 functions code
            USERAPP_Implement05FuncCode( );
        }
        STM32FX_Modbus_WriteParameterFunctionResponse( psrc, ret );
        if (s_logic05.frestore) //Restore 
        {
            USERAPP_GenerateSystemParameters( 1 );
        }
        memset( &s_logic05, 0, sizeof(S_F05CODE_PARA) );    //after process, must clear
        break;
    }
        
    case FUNC10_WRITE:
    case FUNC27_WRITE:
    {
        ret = STM32FX_Modbus_FunctionCode10_27WriteData( psrc );
        if (ret == ModbusOperatorSucceed)
        {
            //Compare orignal data and new data, refresh new data
            if (!USERAPP_ImplementWriteParaFuncCode( *psrc ))
                ret = WriteParameterFailed;
        }
        STM32FX_Modbus_WriteParameterFunctionResponse( psrc, ret );
        if (s_refflg.flgcommp)
        {
            s_refflg.flgcommp = 0;
            USERAPP_GenerateSystemParameters( 1 );
        }
        else
            USERAPP_GenerateSystemParameters( 0 );
        break;
    }
        
    case FUNC25_WRITE:
        ret = STM32FX_Modbus_FunctionCode05_25WriteData( psrc );
        if (ret == ModbusOperatorSucceed)
        {
            //Todo:Process 25 functions code
            if (!USERAPP_Implement25FuncCode( psrc ))
                ret = WriteParameterFailed;
        }
        
        //Response
        STM32FX_Modbus_WriteParameterFunctionResponse( psrc, ret );
        if (s_refflg.flgcalib == MAGNETAPP_CALIBRATION_OK)
        {
            s_refflg.flgcalib = 0;
            USERAPP_GenerateSystemParameters( 0 );
        }
        else if (upgradeflg == 0xFF)
        {
            upgradeflg = 0;
            while (STM32FX_Modbus_ModbusIsSendingData())
            {
#ifdef USE_IDWDG
            FEED_I_WATCHDOG();
#endif
            }
            //Restart system and upgrade
            __disable_irq();
            NVIC_SystemReset( );
        }
        memset( &s_logic25, 0, sizeof(S_F25CODE_PARA) );    //after process, must clear
        break;
        
    case FUNC2A_WRITE:
        ret = STM32FX_Modbus_FunctionCode2AWriteInformation( psrc );
        if (ret == ModbusOperatorSucceed)
        {
            if (!USERAPP_Implement2AFuncCode( psrc + 5 ))
            {
                ret = WriteParameterFailed;
            }
        }
        STM32FX_Modbus_WriteParameterFunctionResponse( psrc, ret );
        break;
    }
}




/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
static bool USERAPP_Implement05FuncCode( void )
{
    bool ret = true;
    u8 cmd = FUNC05_WRITE;
    u32 opdestaddr;
    u32 tempbuf[FLASH_PARA_DW_SIZE];
    memset( (u8*)tempbuf, 0, FLASH_PARA_BUFFER_SIZE );
    if (s_logic05.fbackup == SOWAY_LOGIC_TRUE)
    {
        opdestaddr = SOWAYFLASH_OP_BASE_ADDR;
    }
    else if(s_logic05.frestore == SOWAY_LOGIC_TRUE)
    {
        opdestaddr = SOWAYFLASH_BKUPPARA_ADDR;
    }
    
    do{
        if (!FLASHOP_ReadDataFromChipFlash( (u8*)tempbuf, opdestaddr, FLASH_PARA_DW_SIZE, FLASH_PARA_DW_SIZE ))
        {
            ret = false;
            break;
        }
        if (!USERAPP_WriteParameter2Flash( cmd, (u8*)tempbuf ))
        {
            ret = false;
            break;
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
static bool USERAPP_Implement25FuncCode( u8* psrc )
{
    bool ret = true;
    
    if (!psrc)
        return false;
    
    if (s_logic25.fcalibrate)
    {
        if (s_logic25.fcalibrate == SOWAY_LOGIC_FALSE)
        {
            s_snrval.zerotmv = s_output_para.tdc_origv;
            s_refflg.flgcalib |= MAGNETAPP_CALIBRATION_ZERO;
        }
        else if(s_logic25.fcalibrate == SOWAY_LOGIC_TRUE)
        {
            s_snrval.fulltmv = s_output_para.tdc_origv;
            s_refflg.flgcalib |= MAGNETAPP_CALIBRATION_FSS;
        }
    }
    else if (s_logic25.fupgrade)
    {
        if (s_logic25.fupgrade == SOWAY_LOGIC_FALSE)
        {
            upgradeflg = 0;
        }
        else if (s_logic25.fupgrade == SOWAY_LOGIC_TRUE)
        {
            upgradeflg = 0xFF;
        }
        s_logic25.fupgrade = 0;
    }
    else
        ret = false;
    
    if ((s_refflg.flgcalib == MAGNETAPP_CALIBRATION_OK) || (upgradeflg == 0xFF))
    {
        USERAPP_ImplementWriteParaFuncCode( FUNC25_WRITE );
    }
    
    return ret;
}



/***************************************************************************************
Name:  
Func:   
Para:
Retn:
***************************************************************************************/
static bool USERAPP_Implement2AFuncCode( u8* psrc )
{
    u16 off, cnt;
    bool ret = true;
    u32 tempbuf[FLASH_PARA_DW_SIZE];
    
    do {
        if (!psrc)
            ret = false;
    
        memset( (u8*)tempbuf, 0, FLASH_PARA_BUFFER_SIZE );
        if (!FLASHOP_ReadDataFromChipFlash( (u8*)tempbuf, SOWAYFLASH_OP_BASE_ADDR, FLASH_PARA_DW_SIZE, FLASH_PARA_DW_SIZE ))
        {
            ret = false;
            break;
        }
        off = (*psrc ++)<< FUNC2A_EXPT;
        cnt = (*psrc ++)<< FUNC2A_EXPT;
        memcpy( ((u8*)tempbuf) + off, psrc, cnt );
    }while (0);
    
    if (!USERAPP_WriteParameter2Flash( FUNC2A_WRITE, (u8*)tempbuf ))
        ret = false;
     
    return ret;
}




/***************************************************************************************
Name:  
Func:   
Para:
Retn:
***************************************************************************************/
static bool USERAPP_ImplementWriteParaFuncCode( u8 cmd )
{
    u8* ptr;
    bool re = true;
    bool ref = false;
    u32 tempbuf[FLASH_PARA_DW_SIZE];
    
    memset( (u8*)tempbuf, 0, FLASH_PARA_BUFFER_SIZE );
    ptr = (u8*)tempbuf + USERPARA_INFO_OFFSET;
    do{
        if (!FLASHOP_ReadDataFromChipFlash( (u8*)tempbuf, SOWAYFLASH_OP_BASE_ADDR, FLASH_PARA_DW_SIZE, FLASH_PARA_DW_SIZE ))
        {
            re = false;
            break;
        }
        
        switch (cmd)
        {
            case FUNC10_WRITE:  //10 Function code
            {
                //Slave ID
                if (ptr[OFF_SLAVE_ID] != s_combine.edv_addr)
                {
                    if (s_combine.edv_addr)
                    {
                        ptr[OFF_SLAVE_ID] = (u8)s_combine.edv_addr;
                        s_refflg.flgcommp = 1;
                        ref = true;
                    }
                }
                
                //Baud rate
                if (ptr[OFF_BAUDRATE] != s_combine.baud_rate)
                {
                    if (s_combine.baud_rate < 8)
                    {
                        ptr[OFF_BAUDRATE] = s_combine.baud_rate;
                        s_refflg.flgcommp = 2;
                        ref = true;
                    }
                }
                
                //Parity
                if (ptr[OFF_PARITY] != s_combine.oe_parity)
                {
                    if (s_combine.oe_parity < 4)
                    {
                        ptr[OFF_PARITY] = s_combine.oe_parity;
                        s_refflg.flgcommp = 3;
                        ref = true;
                    }
                }
                
                //Modbus data type
                if (ptr[OFF_MODBUSFT] != s_combine.modbusfmt)
                {
                    if ((s_combine.modbusfmt == 1) || (s_combine.modbusfmt == 2))
                    {
                        ptr[OFF_MODBUSFT] = s_combine.modbusfmt;
                        ref = true;
                    }
                }
                
                //Output mode
                if (ptr[OFF_ANALOGMODE] != s_combine.analog_mode)
                {
                    if (s_combine.analog_mode < ANALOG_OUTPUT_MAXNUM)
                    {
                        ptr[OFF_ANALOGMODE] = s_combine.analog_mode;
                        ref = true;
                    }
                }
                
                //Direction
                if (ptr[OFF_DIRCTION] != s_combine.direction_sel)
                {
                    if (s_combine.direction_sel < 2)
                    {
                        ptr[OFF_DIRCTION] = s_combine.direction_sel;
                        ref = true;
                    }
                }
                break;
            }
        
            case FUNC27_WRITE://27 Function code
            {
                u8 rings;
                u32 val;
                
                //Magnet rings
                rings = (u8)(s_snrval.magnetnum >> 16);
                if (ptr[OFF_MAGNETNUM] != rings)
                {
                    if (rings < 3)
                    {
                        ptr[OFF_MAGNETNUM] = rings;
                        ref = true;
                    }
                }
                
                //PWC Value
                val = ptr[OFF_PWCVALUE] << 8;
                val |= ptr[OFF_PWCVALUE + 1];
                val <<= 16;
                if (val != s_snrval.pwc_val)
                {
                    val = s_snrval.pwc_val >> 16;
                    COMMON_Bits16Convert2Bits8( (ptr + OFF_PWCVALUE), val, BIGENDDIAN );
                    ref = true;
                }
                
                //FSS 
                COMMON_Bits8Convert2Bits32( &val, ptr + OFF_FSSRANGE, BIGENDDIAN );
                if (val != s_snrval.range)
                {
                    COMMON_Bits32Convert2Bits8( (ptr + OFF_FSSRANGE), s_snrval.range, BIGENDDIAN );
                    ref = true;
                }

                //OFF_ZEROPOINT
                COMMON_Bits8Convert2Bits32( &val, ptr + OFF_ZEROPOINT, BIGENDDIAN );
                if (val != s_snrval.zerotmv)
                {
                    COMMON_Bits32Convert2Bits8( (ptr + OFF_ZEROPOINT), s_snrval.zerotmv, BIGENDDIAN );
                    ref = true;
                }
                
                //OFF_FULLPOINT
                COMMON_Bits8Convert2Bits32( &val, ptr + OFF_FULLPOINT, BIGENDDIAN );
                if (val != s_snrval.fulltmv)
                {
                    COMMON_Bits32Convert2Bits8( (ptr + OFF_FULLPOINT), s_snrval.fulltmv, BIGENDDIAN );
                    ref = true;
                }
                break;
            }
            
            case FUNC25_WRITE:
            {
                COMMON_Bits32Convert2Bits8( (ptr + OFF_ZEROPOINT), s_snrval.zerotmv, BIGENDDIAN );
                COMMON_Bits32Convert2Bits8( (ptr + OFF_FULLPOINT), s_snrval.fulltmv, BIGENDDIAN );
                ptr[OFF_UPGRADE] = upgradeflg;
                ref = true;
                break;
            }
        }
        
        if (ref)
        {
            if (!USERAPP_WriteParameter2Flash( cmd, (u8*)tempbuf ))
                re = false;
        }
    }while(0);
    
    return re;
}



/***************************************************************************************
Name:  
Func:   
Para:
Retn:
***************************************************************************************/
static bool USERAPP_WriteParameter2Flash( u8 cmd, u8* psrc )
{
    u8 fwrite = 0;
    u32 flashaddr;
    bool ret = true;
    
    if (!psrc)
        return false;

    switch (cmd)
    {
        case FUNC10_WRITE:
        case FUNC25_WRITE:
        case FUNC27_WRITE:
        case FUNC2A_WRITE:
            fwrite = 1;
            break;

        case FUNC05_WRITE:
        {
            if (s_logic05.fbackup == SOWAY_LOGIC_TRUE)
            {
                fwrite = 2;
            }
            else if(s_logic05.frestore == SOWAY_LOGIC_TRUE)
            {
                fwrite = 1;
            }
            break;
        }
        
        default:
            fwrite = 1;
            break;
    }

    if (fwrite == 1)
    {
        flashaddr = SOWAYFLASH_OP_BASE_ADDR;
    }
    else if (fwrite == 2)
    {
        flashaddr = SOWAYFLASH_BKUPPARA_ADDR;
    }
    
    if (FLASHOP_EraseSpecifiedFlashPages( flashaddr, 1 ))
    {
        if (!FLASHOP_WriteData2SpecifiedFlash( flashaddr, (u32*)psrc, FLASH_PARA_DW_SIZE))
        {
            ret = false;
        }
    }
    
    return ret;
}


//--------------------------------------------------------------------------------------
/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
static void USERAPP_ExtInterruptConfig(void)
{
    EXTI_InitTypeDef   EXTI_InitS;
    GPIO_InitTypeDef   GPIO_InitS;
    NVIC_InitTypeDef   NVIC_InitS;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);  //Enable SYSCONFIG clock
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);
    
    //PA0:TDC STOP input signal pin;
    RCC_AHBPeriphClockCmd(ANA_STOP_CLK_RCC, ENABLE);
    GPIO_InitS.GPIO_Pin = ANA_STOP_SRC_PIN;
    GPIO_InitS.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitS.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitS.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_InitS.GPIO_OType = GPIO_OType_OD;
	GPIO_Init(ANA_STOP_SRC_PORT, &GPIO_InitS);
    GPIO_ResetBits(ANA_STOP_SRC_PORT, ANA_STOP_SRC_PIN);    //Reset tdc_stop pin
    
    EXTI_InitS.EXTI_Line = ANA_STOP_EXTI_LINE;
	EXTI_InitS.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitS.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitS.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitS);
    ANA_STOP_EXTI_DISABLE();
    
    //TDC STOP interrupt NVIC init
    NVIC_InitS.NVIC_IRQChannel = ANA_STOP_EXTI_CHNL;
    NVIC_InitS.NVIC_IRQChannelCmd = ENABLE;
    
    //According magnet number assign priority
    NVIC_InitS.NVIC_IRQChannelPriority = 1;
    NVIC_Init(&NVIC_InitS);
    
    //PB8:TDC interrupt input signal pin
    RCC_AHBPeriphClockCmd(TDC_INTN_CLK_RCC, ENABLE);
    GPIO_InitS.GPIO_Pin = TDC_INTN_SRC_PIN;
    GPIO_InitS.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitS.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitS.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitS.GPIO_OType = GPIO_OType_OD;
	GPIO_Init(TDC_INTN_SRC_PORT, &GPIO_InitS);
    
    //Set TDC_INTN pin
    GPIO_SetBits(TDC_INTN_SRC_PORT, TDC_INTN_SRC_PIN);
    SYSCFG_EXTILineConfig(TDC_INTN_EXTI_PORTSRC, TDC_INTN_TXTI_PINSRC);
    EXTI_InitS.EXTI_Line = TDC_INTN_EXTI_LINE;
	EXTI_InitS.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitS.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitS.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitS);
    TDC_INTN_EXTI_DISABLE();
    
    NVIC_InitS.NVIC_IRQChannel = TDC_INTN_EXTI_CHNL;
    NVIC_InitS.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitS.NVIC_IRQChannelPriority = 2;
    NVIC_Init(&NVIC_InitS);
    TDC_INTN_EXTI_ENABLE();
}


//Interrupt service program segment
/***************************************************************************************
Name:  
Func:   
Para:
Retn:
***************************************************************************************/
void EXTI0_1_IRQHandler( void )
{
    if(EXTI_GetITStatus(EXTI_Line0) != RESET)
	{
        DISABLE_ENSTOP();           //Disable TDC sample stop signal
        COMMON_DelayXus( 20 );      //Delay 20 us
        ENABLE_ENSTOP();            //Enable TDC sample stop signal
		EXTI_ClearITPendingBit(EXTI_Line0);
	}
}


/***************************************************************************************
Name:  
Func:   
Para:
Retn:
***************************************************************************************/
void EXTI4_15_IRQHandler( void )
{
    u8 i;
    unsigned long long sum;
    
    s_snrref.magnetsta = 0;
    if(EXTI_GetITStatus(TDC_INTN_EXTI_LINE) != RESET)
	{
        do{
            //Read data1 from TDC
            s_tdcdat1.data_array[s_tdcdat1.curpos] = TDC_Read32BitsDataFromTDC( ADDR_TDCDATA0 );
            s_tdcdat1.curpos = (s_tdcdat1.curpos + 1) % FILTER_NUM;
            
            if (trig_flg == 1)
            {
                trig_flg = 2;
                for ( i = 1; i < FILTER_NUM; i ++)
                {
                    s_tdcdat1.data_array[i] = s_tdcdat1.data_array[0];
                }
            }

            sum = 0;
            for (i = 0; i < FILTER_NUM; i ++)
            {
               sum += s_tdcdat1.data_array[i];
            }
            s_tdcdat1.tdc_avg = sum >> FILTER_LEVEL;
            MAGNETAPP_ProcessSensorData( s_tdcdat1.tdc_avg, 1 );
            
            if (u8magnetnum < 2)   //Magnet num more than 1
                break;
            
            //Read data2 from TDC
            TDC_Write_TDCRegister( ADDR_TDCCFG1, 0x314300 ); 
            COMMON_DelayXus( 2 );
            
            s_tdcdat2.data_array[s_tdcdat2.curpos] = TDC_Read32BitsDataFromTDC( ADDR_TDCDATA1 );
            s_tdcdat2.curpos = (s_tdcdat2.curpos + 1) % FILTER_NUM;
            
            if (trig_flg == 2)
            {
                trig_flg = 3;
                for ( i = 1; i < FILTER_NUM; i ++)
                {
                    s_tdcdat2.data_array[i] = s_tdcdat2.data_array[0];
                }
            }
            
            sum = 0;
            for (i = 0; i < FILTER_NUM; i ++)
            {
               sum += s_tdcdat2.data_array[i];
            }
            s_tdcdat2.tdc_avg = sum >> FILTER_LEVEL;
            MAGNETAPP_ProcessSensorData( s_tdcdat2.tdc_avg, 2 );
            
            //Todo: Added the third magnet  //Maxnum magnet num is 3
        }while (0);

        EXTI_ClearITPendingBit( TDC_INTN_EXTI_LINE );
	}
}


/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
static void USERAPP_ADG779ControlTimerConfig( void )
{
    u32 tmp;
    
	TIM_TimeBaseInitTypeDef s_timer; 
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
    
//    tmp = ((u32)s_snrval.pwc_val) >> 16;
//    tmp = ((u32)s_snrval.pwc_val) << 1;
    tmp = ((u32)s_snrval.pwc_val) >> 15;
    tmp /= 5;
	s_timer.TIM_Prescaler = 47;     //	1MHz 1us
  	s_timer.TIM_Period = (u16)tmp;  //	200us
  	s_timer.TIM_CounterMode = TIM_CounterMode_Up;
  	s_timer.TIM_ClockDivision = TIM_CKD_DIV1;
  	TIM_TimeBaseInit(TIM3, &s_timer);
  
  	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPriority = 0x03;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);

  	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
  	TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
  	TIM_Cmd(TIM3,DISABLE);
  	TIM_SetCounter(TIM3, 0);
}


/***************************************************************************************
Name:  
Func:   
Para:
Retn:
***************************************************************************************/
void TIM3_IRQHandler( void )
{
    if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        ADG779_DISABLE();
        TIM_Cmd(TIM3,DISABLE);
        TIM_SetCounter(TIM3, 0);
        TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
	}
}


/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
static void TIM2_Configuration( void )
{
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure; 
  NVIC_InitTypeDef NVIC_InitStructure;
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
  
  TIM_TimeBaseStructure.TIM_Prescaler = 479;  //	100KHz 10us
  TIM_TimeBaseStructure.TIM_Period = 200 ;    //    count to 200 interrupt(200 * 10us)
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
  
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 0x00;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
  TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
  TIM_Cmd(TIM2,DISABLE);
  TIM_SetCounter(TIM2, 0);
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
  TIM_Cmd(TIM2,ENABLE);
}




/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
void TIM2_IRQHandler( void )
{
    if (s_snrref.magnetsta)
    {
        s_snrref.deadareaflg = 3;
    }
    s_snrref.magnetsta = 1;
    
    if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);  
        
        ANA_STOP_EXTI_DISABLE();    //Disable stop interrupt
        DISABLE_ENSTOP();           //Disable TDC stop

        if (u8magnetnum == 2)
        {
            TDC_Write_TDCRegister( ADDR_TDCCFG1, 0x214300 );
        }
        TDC_WriteCommand2Tdc( CMD_TDC_INIT );
        
        //Generator 2 us pulse
        DISABLE_START();
        ENABLE_START();
        COMMON_DelayXus( 2 );
        DISABLE_START();
        
        //ADG779
        TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);  //Enalbe ADG779 timer
        TIM_Cmd(TIM3,ENABLE);
        ADG779_ENABLE();
        //Timer ADG779
        
        COMMON_DelayXus(15);
        
        ANA_STOP_EXTI_ENABLE();
        ENABLE_ENSTOP();
    }
    s_snrref.magnetsta = 3;
}


/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
static void MAGNETAPP_ProcessSensorData( u32 tdc_avg, u8 index )
{
    u32 normalval;
    volatile u32 detatime;
    volatile unsigned long long tmp;
    union{
        u32     udigv;
        float   fdigiv;
    }outv;
    
    s_output_para.tdc_origv = tdc_avg;
    
    if (tdc_avg > s_snrval.fulltmv )
    {
        detatime = u32_t_interval;
        s_snrref.deadareaflg = 2;
    }
    else if (tdc_avg < s_snrval.zerotmv)
    {
        detatime = 0;
        s_snrref.deadareaflg = 1;
    }
    else
    {
        detatime = tdc_avg - s_snrval.zerotmv;
        s_snrref.deadareaflg = 0;
    }
    
    //According direction set output
    if (s_combine.direction_sel == 1)   //Reverse
    {
        detatime = u32_t_interval - detatime;
    }
    
    //----------------------------------------------------------------------------------
    tmp = detatime;
    tmp *= (s_snrval.range >> 16);
    outv.fdigiv = (float)((tmp * 1000) / u32_t_interval);
    outv.fdigiv /= 1000;
    
    tmp = detatime;
    tmp <<= 18;
    tmp /= u32_t_interval;
    tmp *= s_snrval.range;
    tmp >>= 18;
    
    normalval = (u32)(((tmp & 0xFFFF) * 65535) >> 16);
    detatime = (u32)(tmp & 0xFFFF0000);
    normalval |= detatime;
    
    if (index == 1)
    {
        s_output_para.position1 = normalval;
        s_output_para.position3 = outv.udigv;
    }
//    else if (index == 2)
//    {
//        s_output_para.position2 = digiv;
//    }
    
    if ((s_combine.analog_mode) && (s_combine.analog_mode < 7))
    {
        tmp = u64_ad_facroty * detatime;
        tmp >>= 32;
        ANALOG_RefreshAD5422Output( (u16)tmp, index );
    }
}

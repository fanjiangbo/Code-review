/***************************************************************************************
                    This file is the Modbus database source file

                Edited by xie_yuyin 2018-03-08 for stm32f072 base version
***************************************************************************************/
#ifndef __MODBUSDATABASE_H
#define __MODBUSDATABASE_H

//Include files


//Magnet parameter
//Todo: Modify relevant item value to accord with specified production

//-------------------Modbus Func03 code address range list----**************************
#define FUNC03_STAR_ADDR_MIN    0x0030
#define FUNC03_STAR_ADDR_MAX    0x0060

//------------Modbus Func04 code address range list-----********************************
#define FUNC04_STAR_ADDR_MIN    0x0000
#define FUNC04_STAR_ADDR_MAX    0x0088
#define FUNC04_REG_MAXNUM       0x007D


//Func 10 code *************************************************************************
#define NUM_F10_VARIABLE NUM_F03_VARIABLE
#define Func10CodeAddrTab   Func03CodeAddrTab

//#define NUM_FUNC10_VARIABLE 18
//const u16 Func10CodeAddrTab[NUM_FUNC10_VARIABLE] = {
//    END_DEVICE_ADDR,    COMM_BAUD_RATE, Odd_Even_Parit, Reserve_Item,   Linear_Comp_en, Filter_Ratio,
//    AutoUploadTime,     Temp_Comp_K,    Temp_Comp_B,    Pos_Alarm_Thd1, Return_Diff1,   Pos_Alarm_Thd2,
//    Return_Diff2,       Convert_Unit,   Director_Select,TDC_UpdateTime, OLED_Select,    Output_Mode };


//Func 26 code**************************************************************************
#define FUNC26_STAR_ADDR_MAX    0x00B2
#define NUM_F26_VARIABLE    7


//Func27 code***************************************************************************
#define FUNC27_STAR_ADDR_MAX    0x00B2
#define NUM_F27_VARIABLE    7

#define F27_SENSOR_RANGE    0x0080
#define F27_SENSOR_ZEROTMV  0x0082
#define F27_SENSOR_FULLTMV  0x0084
#define F27_SENSOR_TMPCMPK  0x0090
#define F27_SENSOR_TMPCMPB  0x0092
#define F27_SENSOR_PWCVAL   0x00B0
#define F27_SENSOR_MAG_NUM  0x00B2
const u16 Func27CodeAddrTab[NUM_F27_VARIABLE] = { F27_SENSOR_RANGE, F27_SENSOR_ZEROTMV, F27_SENSOR_FULLTMV, 
    F27_SENSOR_TMPCMPK, F27_SENSOR_TMPCMPB, F27_SENSOR_PWCVAL, F27_SENSOR_MAG_NUM };

//Func2A code***************************************************************************
#define FUNC2A_START_ADDR   0x00E0


//Func2A code***************************************************************************
#define FUNC2B_START_ADDR_MAX   0x00E3
#define NUM_F2B_VARIABLE    4

#define F2B_READ_APP_VERSION    0x00E0
#define F2B_READ_MBLIB_VERSION  0x00E1
#define F2B_READ_EELIB_VERSION  0x00E2
#define F2B_READ_IICLIB_VERSION 0x00E3
const u16 Func2BCodeAddrTab[NUM_F27_VARIABLE] = {F2B_READ_APP_VERSION, F2B_READ_MBLIB_VERSION,
    F2B_READ_EELIB_VERSION, F2B_READ_IICLIB_VERSION };


//**************************************************************************************
#ifndef NUM_F03_VARIABLE
#define NUM_F03_VARIABLE    1
#endif

#ifndef NUM_F04_VARIABLE
#define NUM_F04_VARIABLE    1
#endif

#ifndef NUM_F05_VARIABLE
#define NUM_F05_VARIABLE    1
#endif

#ifndef NUM_F10_VARIABLE
#define NUM_F10_VARIABLE    1
#endif

#ifndef NUM_F25_VARIABLE
#define NUM_F25_VARIABLE    1
#endif


#ifndef NUM_F26_VARIABLE
#define NUM_F26_VARIABLE    1
#endif

#ifndef NUM_F27_VARIABLE
#define NUM_F27_VARIABLE    1
#endif


#ifndef NUM_F2A_VARIABLE
#define NUM_F2A_VARIABLE    1
#endif

#ifndef NUM_F2B_VARIABLE
#define NUM_F2B_VARIABLE    1
#endif


#endif

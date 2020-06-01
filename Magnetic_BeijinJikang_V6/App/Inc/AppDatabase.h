/***************************************************************************************
                    This file is the Modbus database header file

                Edited by xie_yuyin 2019-05-08 for stm32f072 base version
***************************************************************************************/
#ifndef __APPDATABASE_H
#define __APPDATABASE_H
//Include files
#include    "Baseincludes.h"

//Func03 code dest address list ********************************************************
#define END_DEVICE_ADDR 0x0030
#define COMM_BAUD_RATE  0x0031
#define ODD_EVEN_PARIT  0x0032
#define MODBUS_DATATYPE 0x0033
#define LINEAR_COMP_EN  0x0034
#define FILTER_RATIO    0x0035
#define AUTOUPLOADTIME  0x0036
#define TEMP_CMOP_K     0x0037
#define TEMP_CMOP_B     0x0038
#define POS_ALARM_THD1  0x0039
#define RETURN_DIFF1    0x003A
#define POS_ALARM_THD2  0x003B
#define RETURN_DIFF2    0x003C
#define CONVERT_UNIT    0x003D
#define DIRECTOR_SELECT 0x003E
#define TDC_UPDATETIME  0x003F
#define OLED_SELECT     0x0042
#define OUTPUT_MODE     0x0060
 
const u16 Func03CodeAddrTab[] = {
    END_DEVICE_ADDR,    COMM_BAUD_RATE, ODD_EVEN_PARIT, MODBUS_DATATYPE,    LINEAR_COMP_EN,
    FILTER_RATIO,       AUTOUPLOADTIME, TEMP_CMOP_K,    TEMP_CMOP_B,        POS_ALARM_THD1,
    RETURN_DIFF1,       POS_ALARM_THD2, RETURN_DIFF2,   CONVERT_UNIT,       DIRECTOR_SELECT,
    TDC_UPDATETIME,     OLED_SELECT,    OUTPUT_MODE };

//Func04 code dest address list ********************************************************
#define MAGNETRING_POS1     0x0000  //Magnet ring position1
#define ENVIRENMENT_TEMP    0x0002  //Environment temprature
#define MAGNETRING_POS2     0x0004  //Magnet ring position2
#define MAGNETRING_POS3     0x0006  //Magnet ring position3
#define MAGNETRING_MAX      0x0040  //Magnet ring upper limited position
#define MAGNETRING_MIN      0x0042  //Magnet ring lower limited position
#define ORIGINAL_TDC_VALUE  0x0080  //TDC original value
#define WAVEGUID_VELOCITY   0x0082  //Waveguid wire velocity

const u16 Func04CodeAddrTab[] = {
    MAGNETRING_POS1,    ENVIRENMENT_TEMP,   MAGNETRING_POS2,    MAGNETRING_POS3,
    MAGNETRING_MAX,     MAGNETRING_MIN,     ORIGINAL_TDC_VALUE, WAVEGUID_VELOCITY };


//Func05 code dest address list ********************************************************
#define CALIBRATION_ZERO    0x0050
#define RESTORE_FACTORRY    0x0051
#define FROZEN_EN_DISABLE   0x0052
#define EEPROM_WRITE_CTRL   0x0053
#define CLEAR_MIN_MAX_LMT   0x0054

const u16 Func05CodeAddrTab[] = {
    CALIBRATION_ZERO, RESTORE_FACTORRY, FROZEN_EN_DISABLE, EEPROM_WRITE_CTRL, CLEAR_MIN_MAX_LMT };


//Func 25 code**************************************************************************
#define F25_CALIBRATION     0x0000
#define F25_ONLINEUPGRADE   0x0040

const u16 Func25CodeAddrTab[] = { F25_CALIBRATION, F25_ONLINEUPGRADE };

//Func26 code dest address list ********************************************************
#define F26_SENSOR_RANGE    0x0080
#define F26_SENSOR_ZEROTMV  0x0082
#define F26_SENSOR_FULLTMV  0x0084
#define F26_SENSOR_TMPCMPK  0x0090
#define F26_SENSOR_TMPCMPB  0x0092
#define F26_SENSOR_PWCVAL   0x00B0
#define F26_SENSOR_MAG_NUM  0x00B2
const u16 Func26CodeAddrTab[] = {
   F26_SENSOR_RANGE, F26_SENSOR_ZEROTMV, F26_SENSOR_FULLTMV,    F26_SENSOR_TMPCMPK,
   F26_SENSOR_TMPCMPB, F26_SENSOR_PWCVAL, F26_SENSOR_MAG_NUM };

//Func2A code dest address list
#define F2A_2B_ITEM_NUM     6
#define F2A_ORGANIZE_CODE   0x00E0
#define F2A_PRODUCTION_CODE 0x00E1
#define F2A_HARDWARE_VER    0x00E2
#define F2A_SOFTWARE_VER    0x00E3
#define F2A_DEVICE_ID       0x00E4
#define F2A_CUSTORMER_ID    0x00E5
const u16 Func2ACodeAddrTab[] = {
    F2A_ORGANIZE_CODE, F2A_PRODUCTION_CODE, F2A_HARDWARE_VER, F2A_SOFTWARE_VER, F2A_DEVICE_ID, F2A_CUSTORMER_ID };

#endif


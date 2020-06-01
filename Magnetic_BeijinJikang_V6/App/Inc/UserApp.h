/***************************************************************************************
                    This file is the Modbus header source file

                Edited by xie_yuyin 2018-05-09 for stm32f072 base version
***************************************************************************************/
#ifndef __MDTEST_H
#define __MDTEST_H
//Include files


//Datatype define
typedef struct{
    u16 edv_addr;
    u16 baud_rate;
    u16 oe_parity;
    u16 modbusfmt;
    u16 linear_comp;
    u16 filter_ratio;
    u16 auto_uploadtime;
    u16 temp_comp_k;
    u16 temp_comp_b;
    u16 alarm_thd_pos1;
    u16 return_diff1;
    u16 alarm_thd_pos2;
    u16 return_diff2;
    u16 conver_unit;
    u16 direction_sel;
    u16 tdc_updatetime;
    u16 oled_select;
    u16 analog_mode;    //output_mode;
//    u16 bgk_edvaddr;    //BeiJingJikang device address
}S_F03CODE_PARA, *PS_F03CODE_PARA;

typedef struct{
    u32 position1;      //magnet position 1
    u32 env_temp;       //environment temprature
    u32 position2;      //magnet position 2
    u32 position3;      //magnet position 3
    u32 upper_lim;      //upper limited position
    u32 lower_lim;      //lower limited position
    u32 tdc_origv;     //orignal tdc value
    u32 wgw_velocity;   //waveguide wire velocity
}S_F04CODE_PARA, *PS_F04CODE_PARA;

typedef struct{
    u8  fcalizero;
    u8  frestore;
    u8  fbackup;
    u8  feepromwp;
    u8  fclrlimited;
}S_F05CODE_PARA, *PS_F05CODE_PARA;

typedef struct{
    u8  fcalibrate;
    u8  fupgrade;
}S_F25CODE_PARA, *PS_F25CODE_PARA;

typedef struct{
    u32 range;
    u32 zerotmv;
    u32 fulltmv;
    u32 tmp_k1;
    u32 tmp_b1;
    u32 pwc_val;
    u32 magnetnum;
}S_F26CODE_PARA, *PS_F26CODE_PARA;


//Parameter offset value in flash
#define OFF_CHIP_INIT   0
#define OFF_UPGRADE     (OFF_CHIP_INIT + 1)
#define OFF_SLAVE_ID    (OFF_UPGRADE + 1)
#define OFF_BAUDRATE    (OFF_SLAVE_ID + 1)
#define OFF_PARITY      (OFF_BAUDRATE + 1)
#define OFF_MODBUSFT    (OFF_PARITY + 1)
#define OFF_ANALOGMODE  (OFF_MODBUSFT + 6)
#define OFF_DIRCTION    (OFF_ANALOGMODE + 1)
#define OFF_MAGNETNUM   (OFF_DIRCTION + 9)
#define OFF_PWCVALUE    (OFF_MAGNETNUM + 7)
#define OFF_FSSRANGE    (OFF_PWCVALUE + 2)
#define OFF_ZEROPOINT   (OFF_FSSRANGE + 4)
#define OFF_FULLPOINT   (OFF_ZEROPOINT + 4)
#define SYSPARA_NUM     (OFF_FULLPOINT + 4)

#define CHIP_INIT_FLAG          0xAA

#define COMPANY_INFO_ITEMNUM    8
#define COMPANY_INFO_SIZE       512

#define USERPARA_INFO_OFFSET    512
#define FLASH_PARA_DW_SIZE      256
#define FLASH_PARA_BUFFER_SIZE  (FLASH_PARA_DW_SIZE << 2)

//**************************************************************************************
typedef struct{
    u8  magnetsta;  //1: normal 2:dead area 3:no magnet ring
    u8  communicte;
    u8  deadareaflg;
    u8  reserve;
}S_SENSOR_REF, *PS_SENSOR_REF;

#define FILTER_LEVEL    6
#define FILTER_NUM      (1 << FILTER_LEVEL)
typedef struct{
    u8  curpos;
    u32 tdc_avg;    //tdc unit counter, original tdc value
    u32 data_array[FILTER_NUM];
}S_TDCDATA_TAG, *PS_TDCDATA_TAG;

//Functions declare
void USERApp_Init( void );
void TIM2_IRQHandler( void );
void TIM3_IRQHandler( void );
void EXTI0_1_IRQHandler( void );
void EXTI4_15_IRQHandler( void );
void USERApp_MagnetProcess( void );

#endif


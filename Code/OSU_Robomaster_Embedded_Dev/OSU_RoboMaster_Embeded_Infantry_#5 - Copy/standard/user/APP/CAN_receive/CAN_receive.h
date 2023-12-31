/**
  ****************************(C) COPYRIGHT 2016 DJI****************************
  * @file       can_receive.c/h
  * @brief      can device transmit and recevice function，receive via CAN interrupt
  * @note       This is NOT a freeRTOS TASK
  * @history
  *  Version    Date            Author          Modification
  *  V1.0.0     Dec-26-2018     RM              Complete
  *  V1.0.1     Feb-17-2019     Tony-OSU        Add tx2 can bus config
	*  V1.1.0     Feb-21-2019     Tony-OSU        Finish Custom CAN Bus, fully functional
	*  V1.2.0     Mar-01-2019     Tony-OSU        Package ID modified 
	*																							@note TX2 package is now 0x111 for higher priority
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  ****************************(C) COPYRIGHT 2016 DJI****************************
  **************Modifid by Ohio State University Robomaster Team****************

  */

#ifndef CANTASK_H
#define CANTASK_H
#include "main.h"

#define CHASSIS_CAN CAN2
#define GIMBAL_CAN CAN1
#define TX2_CAN CAN2
//#define PID_TUNING_CAN CAN2


/* Enumerate CAN send and receive ID */
/* 枚举声明CAN收发ID*/
typedef enum
{
	  CAN_AIM_DATA_ID = 0x300,//自瞄数据ID
	
    CAN_CHASSIS_ALL_ID = 0x200,
    CAN_3508_M1_ID = 0x201,
    CAN_3508_M2_ID = 0x202,
    CAN_3508_M3_ID = 0x203,
    CAN_3508_M4_ID = 0x204,

    CAN_YAW_MOTOR_ID = 0x205,
    CAN_PIT_MOTOR_ID = 0x206,
    CAN_TRIGGER_MOTOR_ID = 0x207,
    CAN_GIMBAL_ALL_ID = 0x1FF,
		
	
		GYRO_DATA_TX2_ID=0x215,//陀螺仪绝对角度数据ID
	  CAN_GIMBAL_YAW_INTER_TRANSFER_ID=0x210, //Transfer Gimbal data to CAN2
	  CAN_GIMBAL_PITCH_INTER_TRANSFER_ID=0x211,
		CAN_TRIGGER_INTER_TRANSFER_ID=0x212//供弹轮ID
} can_msg_id_e;

//RM electrical motor unified data struct
//RM电机统一数据结构体
typedef struct
{
    uint16_t ecd;//encoder
    int16_t speed_rpm;//round per minute
    int16_t given_current;
    uint8_t temperate;
    int16_t last_ecd;
} motor_measure_t;

//TX2 to Gimbal Motor aim coordinate location data package
//TX2到云台电机瞄准坐标数据包
typedef struct{
	//接收的数据
	int32_t raw_horizontal_pixel;
	int32_t raw_vertical_pixel;
	int32_t last_raw_horizontal_pixel;
	int32_t last_raw_vertical_pixel;
	//分析的数据
	int32_t horizontal_pixel;
	int32_t vertical_pixel;
	int32_t horizontal_pixel_difference;
	int32_t vertical_pixel_difference;
} tx2_aim_package_t;

//云台陀螺仪绝对角度结构体
typedef struct
{
	uint16_t absolute_yaw_angle;
	uint16_t absolute_pitch_angle;
} gimbal_gyro_absolute_angle_t;




//发送重设底盘电机ID命令
extern void CAN_CMD_CHASSIS_RESET_ID(void);

//发送云台控制命令，其中rev为保留字节
extern void CAN_CMD_GIMBAL(int16_t yaw, int16_t pitch, int16_t shoot, int16_t rev);
//发送底盘电机控制命令
extern void CAN_CMD_CHASSIS(int16_t motor1, int16_t motor2, int16_t motor3, int16_t motor4);
//发送云台陀螺仪数据
extern void CAN_GIMBAL_GYRO_DATA(int16_t yaw, int16_t pitch);
//发送云台编码器数据
extern void CAN_GIMBAL_ENCODE_DATA(uint8_t *data,int id);
//返回yaw电机变量地址，通过指针方式获取原始数据
extern const motor_measure_t *get_Yaw_Gimbal_Motor_Measure_Point(void);
//返回pitch电机变量地址，通过指针方式获取原始数据
extern const motor_measure_t *get_Pitch_Gimbal_Motor_Measure_Point(void);
//返回trigger电机变量地址，通过指针方式获取原始数据
extern const motor_measure_t *get_Trigger_Motor_Measure_Point(void);
//返回底盘电机变量地址，通过指针方式获取原始数据,i的范围是0-3，对应0x201-0x204,
extern const motor_measure_t *get_Chassis_Motor_Measure_Point(uint8_t i);
//自瞄数据
extern tx2_aim_package_t tx2;
#if GIMBAL_MOTOR_6020_CAN_LOSE_SLOVE
extern void GIMBAL_lose_slove(void);
#endif

#endif

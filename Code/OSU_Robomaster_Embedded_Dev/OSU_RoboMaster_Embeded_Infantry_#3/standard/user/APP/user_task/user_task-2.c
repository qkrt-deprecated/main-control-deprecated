/**
  ****************************(C) COPYRIGHT 2016 DJI****************************
  * @file       user_task.c/h
  * @brief      *一个普通心跳程序，如果设备无错误，绿灯1Hz闪烁,然后获取姿态角*
  *             现用与filter计算线程
  * @note
  * @history
  *  Version    Date            Author          Modification
  *  V1.0.0     Dec-26-2018     RM              1. 完成
  *
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  ****************************(C) COPYRIGHT 2016 DJI****************************
  */

#include "User_Task.h"
#include "main.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#include "led.h"

#include "Detect_Task.h"
#include "INS_Task.h"




//自定义
#include "gimbal_task.h"
#include "filter.h"
#include "CAN_Receive.h"
#include "chassis_task.h"


#define user_is_error() toe_is_error(errorListLength)

#if INCLUDE_uxTaskGetStackHighWaterMark
uint32_t UserTaskStack;
#endif

//姿态角 单位度
fp32 angle_degree[3] = {0.0f, 0.0f, 0.0f};


////自定义内容
//定义结构体，获取数据
Gimbal_Control_t gimbal_control;
tx2_aim_package_t tx2;
int32_t final_angle_set;
//底盘运动数据
chassis_move_t chassis_move;

//声明filter类型
Group_Delay_t group_delay_aim;
Group_Delay_t group_delay_chassis_speed;
Blackman_Filter_t blackman;
IIR_Filter_t butterworth_aim_yaw;
IIR_Filter_t butterworth_aim_pitch;
IIR_Filter_t butterworth_final_angle;

kalman_filter_t kalman;
kalman_filter_init_t kalman_initial;





//定义filter
double Group_Delay(Group_Delay_t *GD);
double Blackman_Filter(Blackman_Filter_t *F);
double Butterworth_Filter(IIR_Filter_t *F);
float kalman_filter_calc(kalman_filter_t *F, float signal1, float signal2);
//extern 滤波后的数据，用于其他文件
extern fp32 delayed_relative_angle;
extern fp32 delayed_wz_set;
extern fp32 filtered_horizontal_pixel;//extern 变量定义类型必须一致
extern int32_t filtered_vertical_pixel;
extern fp32 filtered_final_angle_set;

//声明flag
static int filter_tx2_yaw_data_flag;
static int filter_final_angle_set_flag;


void kalman_filter_init(kalman_filter_t *F, kalman_filter_init_t *I);

static void Filter_Running(Gimbal_Control_t *gimbal_data)
{


    filter_tx2_yaw_data_flag=1;
    filter_final_angle_set_flag=0;//没什么效果




    //是否对tx2发来的yaw轴自瞄数据进行滤波
    if(filter_tx2_yaw_data_flag==1)
    {
        filtered_horizontal_pixel=kalman_filter_calc(&kalman, tx2.horizontal_pixel, 0);
//			filtered_horizontal_pixel=Butterworth_Filter(&butterworth_aim_yaw);//Blackman_Filter(&blackman);
    }
    else if(filter_tx2_yaw_data_flag==0)
    {
        filtered_horizontal_pixel=tx2.horizontal_pixel;
    }

    //是否对tx2发来的pitch轴自瞄数据进行滤波，pitch轴数据无需平滑及预测
//		if(filter_tx2_yaw_data_flag==1)
//		{
//			filtered_horizontal_pixel=Butterworth_Filter(&butterworth_aim_yaw);//Blackman_Filter(&blackman);
//		}
//		else if(filter_tx2_yaw_data_flag==0)
//		{
    filtered_vertical_pixel=tx2.vertical_pixel;
//		}


    //是否对最终角度进行滤波
    if(filter_final_angle_set_flag==1)
    {
        filtered_final_angle_set=Butterworth_Filter(&butterworth_final_angle);
    }
    else if(filter_final_angle_set_flag==0)
    {
        filtered_final_angle_set=final_angle_set;
    }





    delayed_relative_angle=Group_Delay(&group_delay_aim);//经过x ms delay后的编码器的值
    delayed_wz_set=Group_Delay(&group_delay_chassis_speed);
}


void UserTask(void *pvParameters)
{

    const volatile fp32 *angle;
    //获取姿态角指针
    angle = get_INS_angle_point();

//		//Initialize First Esitimate Value
//		kalman_initial.xhat_data[0]=900;
//		kalman_initial.xhat_data[1]=0;
//
//		//Initialize Last State of Prior Estimate Value
//		kalman_initial.xhatminus_data[0]=0;
//		kalman_initial.xhatminus_data[1]=0;
//
//		//Initialize Actual Measurement
//		kalman_initial.z_data[0]=0;
//		kalman_initial.z_data[1]=0;
//
//		//Initialize Kalman Gain Matrix K
//		kalman_initial.K_data[0]=0;
//		kalman_initial.K_data[1]=0;
//		kalman_initial.K_data[2]=0;
//		kalman_initial.K_data[3]=0;
//
//		//Initialize Observation Matrix H
//		kalman_initial.H_data[0]=1;
//		kalman_initial.H_data[1]=0;
//		kalman_initial.H_data[2]=0;
//		kalman_initial.H_data[3]=1;
//
//		//Initialize System Matrix A
//		kalman_initial.A_data[0]=1;
//		kalman_initial.A_data[1]=0;
//		kalman_initial.A_data[2]=0;
//		kalman_initial.A_data[3]=0;
//
//		//Initialize Covariance of the Estimation-Error Matrix P-
//		kalman_initial.Pminus_data[0]=0;
//		kalman_initial.Pminus_data[1]=0;
//		kalman_initial.Pminus_data[2]=0;
//		kalman_initial.Pminus_data[3]=0;
//
//		//Initialize Initial Covariance of the Estimation-Error Matrix P
//		kalman_initial.P_data[0]=0;
//		kalman_initial.P_data[1]=0;
//		kalman_initial.P_data[2]=0;
//		kalman_initial.P_data[3]=0;
//
//		//Initialize Covariance of the Process Noise Matrix Q
//		kalman_initial.Q_data[0]=1;
//		kalman_initial.Q_data[1]=0;
//		kalman_initial.Q_data[2]=0;
//		kalman_initial.Q_data[3]=1;
//
//		//Initialize Covariance of the Measurement Noise Matrix R
//		kalman_initial.R_data[0]=2000;
//		kalman_initial.R_data[1]=0;
//		kalman_initial.R_data[2]=0;
//		kalman_initial.R_data[3]=10000;
//
//		kalman_filter_init(&kalman,&kalman_initial);
    while (1)
    {

        //姿态角 将rad 变成 度，除这里的姿态角的单位为度，其他地方的姿态角，单位均为弧度
        angle_degree[0] = (*(angle + INS_YAW_ADDRESS_OFFSET)) * 57.3f;
        angle_degree[1] = (*(angle + INS_PITCH_ADDRESS_OFFSET)) * 57.3f;
        angle_degree[2] = (*(angle + INS_ROLL_ADDRESS_OFFSET)) * 57.3f;

        if (!user_is_error())
        {
            led_green_on();
        }




        //filter的输入
        group_delay_aim.group_delay_raw_value=gimbal_control.gimbal_yaw_motor.relative_angle;//group delay fir filter输入为编码器相对ecd_offset的角度(-pi/2,pi/2)
        group_delay_chassis_speed.group_delay_raw_value=chassis_move.wz_set;
//				butterworth_aim_yaw.raw_value=tx2.horizontal_pixel;
//				butterworth_final_angle.raw_value=final_angle_set;


        Filter_Running(&gimbal_control);//filter进行计算








        vTaskDelay(1);//每隔1ms循环一次
//        led_green_off();
//        vTaskDelay(500);
#if INCLUDE_uxTaskGetStackHighWaterMark
        UserTaskStack = uxTaskGetStackHighWaterMark(NULL);
#endif
    }
}

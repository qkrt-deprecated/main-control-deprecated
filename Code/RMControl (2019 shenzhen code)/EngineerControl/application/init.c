/****************************************************************************
 *  Copyright (C) 2019 RoboMaster.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of聽
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.聽 See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 ***************************************************************************/

 /****************************************************************************
  *  Analyzed and commented by QueensKnight Robomaster Team
  *  Queen's University in Kingston
  *
  *  Sean
  ***************************************************************************/

//Files are in the library bsp\cubemx\Core\Inc
//Pin assignment and hardware discription
#include "main.h"
#include "can.h"
//The file is in bsp/boards
#include "board.h"
//Files are in components/devices
#include "motor.h"
#include "dbus.h"
#include "detect.h"
//The File is in test folder
#include "test.h"
//Files are in componets/modules
#include "chassis.h"
#include "gimbal.h"
#include "shoot.h"

//Files are in application
#include "chassis_task.h"
#include "gimbal_task.h"
#include "timer_task.h"
#include "shoot_task.h"
#include "rc_light.h"
#include "communicate.h"
#include "infantry_cmd.h"
#include "init.h"

//The file are in applicaton/protocol
#include "protocol.h"
//the file are in utilities/ulog
#include "ulog.h"
//Files are in application
#include "param.h"
#include "offline_check.h"
#include "referee_system.h"

//Init all structure
struct chassis chassis;
struct gimbal gimbal;
struct shoot shoot;
static struct rc_device rc_dev;

//unsigned char, global system config
static uint8_t glb_sys_cfg;

extern int ulog_console_backend_init(void);

//SYS_CFG_Port and SYS_CFG_Pin are defined in main.h for system config
void system_config(void)
{
  glb_sys_cfg = HAL_GPIO_ReadPin(SYS_CFG_Port, SYS_CFG_Pin);
}

//getter for global system config char
uint8_t get_sys_cfg(void)
{
  return glb_sys_cfg;
}

//
void hw_init(void)
{
  //Read calibrated data from encoders and sensors,
  cali_param_init();
  board_config();
  test_init();
  system_config();
  ulog_init();
  ulog_console_backend_init();

  referee_param_init();
  usart3_rx_callback_register(referee_uart_rx_data_handle);
  referee_send_data_register(usart3_transmit);
	
  rc_device_register(&rc_dev, "uart_rc", 0);
  dr16_forword_callback_register(rc_data_forword_by_can);
  chassis_pid_register(&chassis, "chassis", DEVICE_CAN1);
  chassis_disable(&chassis);
  gimbal_cascade_register(&gimbal, "gimbal", DEVICE_CAN1);
  shoot_pid_register(&shoot, "shoot", DEVICE_CAN1);
  gimbal_yaw_disable(&gimbal);
  gimbal_pitch_disable(&gimbal);
  shoot_disable(&shoot);
  offline_init();
}

//define Threads for tasks
osThreadId timer_task_t;
osThreadId chassis_task_t;
osThreadId gimbal_task_t;
osThreadId communicate_task_t;
osThreadId cmd_task_t;
osThreadId shoot_task_t;
osThreadId rc_light_t;

void task_init(void)
{
  uint8_t app;
  app = get_sys_cfg();

  osThreadDef(TIMER_1MS, timer_task, osPriorityHigh, 0, 512);
  timer_task_t = osThreadCreate(osThread(TIMER_1MS), NULL);

  osThreadDef(COMMUNICATE_TASK, communicate_task, osPriorityHigh, 0, 4096);
  communicate_task_t = osThreadCreate(osThread(COMMUNICATE_TASK), NULL);

  osThreadDef(CMD_TASK, infantry_cmd_task, osPriorityNormal, 0, 4096);
  cmd_task_t = osThreadCreate(osThread(CMD_TASK), NULL);

  osThreadDef(RC_LIGHT, rc_light, osPriorityRealtime, 0, 4096);
  rc_light_t = osThreadCreate(osThread(RC_LIGHT), NULL);

  osThreadDef(CHASSIS_TASK, chassis_task, osPriorityRealtime, 0, 512);
  chassis_task_t = osThreadCreate(osThread(CHASSIS_TASK), NULL);
 
//  osThreadDef(GIMBAL_TASK, gimbal_task, osPriorityRealtime, 0, 512);
//  gimbal_task_t = osThreadCreate(osThread(GIMBAL_TASK), NULL);
//  
//  osThreadDef(SHOOT_TASK, shoot_task, osPriorityNormal, 0, 512);
//  shoot_task_t = osThreadCreate(osThread(SHOOT_TASK), NULL);
}


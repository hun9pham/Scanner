#ifndef __TASK_LIST_H
#define __TASK_LIST_H

#include "ak.h"
#include "task.h"
#include "message.h"

#define MPU_SERIAL_INTEFACE_EN	(0)

extern const task_t app_task_table[];
extern task_polling_t app_task_polling_table[];

/*---------------------------------------------------------------------------*
 *  DECLARE: Internal Task ID
 *  Note: Task id MUST be increasing order.
 *---------------------------------------------------------------------------*/
enum {
	SL_TASK_TIMER_TICK_ID,
	SL_TASK_SYSTEM_ID,
	SL_TASK_CONSOLE_ID,
	SL_TASK_DEVICEMANAGER_ID,
	SL_TASK_SENSOR_ID,

#if MPU_SERIAL_INTEFACE_EN
	SL_TASK_IF_ID,
	SL_TASK_CPU_SERIAL_IF_ID,
	SL_LINK_PHY_ID,
	SL_LINK_MAC_ID,
	SL_LINK_ID,
#endif
	SL_TASK_EOT_ID,
};

/*----------------------------------------------------------------------------
 *  DECLARE: Internal Polling Task ID
 *  Note: Task id MUST be increasing order.
 *---------------------------------------------------------------------------*/
enum {
	SL_TASK_POLL_CONSOLE_ID,
#if MPU_SERIAL_INTEFACE_EN
	SL_TASK_POLL_SERIAL_IF_ID,
#endif
	SL_TASK_POLLING_EOT_ID,
};

/*----------------------------------------------------------------------------
 *  DECLARE: Task entry point
 *  
 *---------------------------------------------------------------------------*/
extern void TaskSystem(ak_msg_t *);
extern void TaskConsole(ak_msg_t *);
extern void TaskDeviceManager(ak_msg_t *);
extern void TaskSensor(ak_msg_t *);

#if MPU_SERIAL_INTEFACE_EN
extern void TaskCpuSerialIf(ak_msg_t *);
extern void TaskIf(ak_msg_t *);
extern void TaskLinkPhy(ak_msg_t *);
extern void TaskLinkMac(ak_msg_t *);
extern void TaskLink(ak_msg_t *);
#endif

/*----------------------------------------------------------------------------
 *  DECLARE: Task polling
 *  
 *---------------------------------------------------------------------------*/
extern void TaskPollConsole(void);
#if MPU_SERIAL_INTEFACE_EN
extern void TaskPollSerialIf(void);
#endif

#endif /* __TASK_LIST_H */


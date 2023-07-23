#include "task_list.h"
#include "timer.h"

const task_t app_task_table[] = {
	{SL_TASK_TIMER_TICK_ID		,	TASK_PRI_LEVEL_7	,	task_timer_tick		},
	{SL_TASK_SYSTEM_ID			,	TASK_PRI_LEVEL_6	,	TaskSystem			},
	{SL_TASK_CONSOLE_ID			,	TASK_PRI_LEVEL_3	,	TaskConsole			},
	{SL_TASK_DEVMANAGER_ID		,	TASK_PRI_LEVEL_5	,	TaskDeviceManager	},
	{SL_TASK_EOT_ID				,	TASK_PRI_LEVEL_0	,	(pf_task)0			}
};

task_polling_t app_task_polling_table[] = {
    {SL_TASK_POLL_CONSOLE_ID	,	AK_ENABLE			,	TaskPollConsole      },
	{SL_TASK_POLL_DEVMANAGER_ID	,	AK_DISABLE			,	TaskPollDevManager	 },
	{SL_TAKS_POLL_MPU_IF_ID		,	AK_ENABLE			,	TaskPollMPUIf	     },
	{SL_TASK_POLLING_EOT_ID		,	AK_DISABLE			,	(pf_task_polling)0	 }
};

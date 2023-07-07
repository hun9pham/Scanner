#include <stdbool.h>

#include "ak.h"
#include "message.h"
#include "timer.h"
#include "fsm.h"

#include "app.h"
#include "app_dbg.h"
#include "app_data.h"
#include "task_list.h"
#include "task_sensor.h"

#include "platform.h"
#include "io_cfg.h"
#include "sys_cfg.h"
#include "sys_ctl.h"

#include "sys_dbg.h"

#define TAG	"TaskSensor"

/* Extern variables ----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Function implementation ---------------------------------------------------*/
void TaskSensor(ak_msg_t* msg) {
	switch (msg->sig) {

	default:
	break;
	}
}

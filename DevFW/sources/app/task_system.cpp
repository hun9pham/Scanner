#include <stdbool.h>

#include "ak.h"
#include "message.h"
#include "timer.h"
#include "fsm.h"

#include "app.h"
#include "app_dbg.h"
#include "app_data.h"
#include "app_bsp.h"
#include "task_list.h"
#include "task_system.h"

#include "platform.h"
#include "io_cfg.h"
#include "sys_cfg.h"
#include "sys_ctl.h"

#include "sys_dbg.h"

#define TAG	"TaskSystem"

/* Extern variables ----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Function implementation ---------------------------------------------------*/
void TaskSystem(ak_msg_t* msg) {
	switch (msg->sig) {
	case SL_SYSTEM_ALIVE_NOTIFY: {
		LEDLIFE.Blinking();
		watchdogRst();		
	}
	break;

	case SL_SYSTEM_REBOOT_REQ: {
        APP_DBG_SIG(TAG, "SL_SYSTEM_REBOOT_REQ");
		softReset();
	}
	break;

	default:
	break;
	}
}

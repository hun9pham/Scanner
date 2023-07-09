#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ak.h"
#include "task.h"
#include "message.h"
#include "heap.h"
#include "timer.h"

#include "app.h"
#include "app_dbg.h"
#include "app_data.h"
#include "app_bsp.h"
#include "task_list.h"
#include "task_if.h"
#include "task_cpu_serial_if.h"
#include "task_deviceManager.h"
#include "task_console.h"

#include "io_cfg.h"
#include "sys_cfg.h"
#include "sys_ctl.h"
#include "platform.h"

#include "sys_dbg.h"

#define TAG	"App"

/* Extern variables ----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static void app_start_timer();
static void app_init_state_machine();
static void app_task_init();

/* Function implementation ---------------------------------------------------*/
int main_app() {
	APP_PRINT(KBLU"\n"
			"    \\              | _)              |   _)             \n"
			"   _ \\   _ \\  _ \\  |  |   _|   _` |   _|  |   _ \\    \\  \n"
			" _/  _\\ .__/ .__/ _| _| \\__| \\__,_| \\__| _| \\___/ _| _| \n"
			"       _|   _|                                          \n"
			);
	APP_PRINT(KNRM"\n[APPLICATION] Welcome to my application\n");

	/*---------------------------------------------------------------------*/
	/*						Init active kernel 						   	   */	 
	/*---------------------------------------------------------------------*/
	ENTRY_CRITICAL();
	task_init();
	task_create((task_t*)app_task_table);
	task_polling_create((task_polling_t*)app_task_polling_table);
	EXIT_CRITICAL();

	/*---------------------------------------------------------------------*/
	/*					 Software configuration 						   */	 
	/*---------------------------------------------------------------------*/
	watchdogInit();	/* 32s */
	
	/*---------------------------------------------------------------------*/
	/*					 Hardware configuration							   */
	/*---------------------------------------------------------------------*/
	buzzerInit();
	MPUInterfaceInit();
	inputsInit();
	motorsPWMInit();

	LEDLIFE.initialize(ledLifeInit, ledLifeOn, ledLifeOff);
	LEDFLASH.initialize(ledFlashInit, ledFlashOn, ledFlashOff);
	LEDSTATUS.initialize(ledStatusInit, ledStatusOn, ledStatusOff);
	LEDDIR.initialize(ledDirInit, ledDirOn, ledDirOff);
	LEDFAULT.initialize(ledFaultInit, ledFaultOn, ledFaultOff);
	ENGINES.initialize();

	/*---------------------------------------------------------------------*/
	/*							 System app setup						   */	
	/*---------------------------------------------------------------------*/
	fatalInit();

	/*---------------------------------------------------------------------*/
	/*						Application task initial					   */	
	/*---------------------------------------------------------------------*/
	app_init_state_machine();
	app_start_timer();
	app_task_init();

	/*---------------------------------------------------------------------*/
	/*							 System app run							   */
	/*---------------------------------------------------------------------*/
	EXIT_CRITICAL();

	return task_run();
}

/**---------------------------------------------------------------------------*/
/* App initial function.
 */
/*----------------------------------------------------------------------------*/
/*---------------------------------------
 * Start software timer for application
 * used for app tasks
 ---------------------------------------*/
void app_start_timer() {
    timer_set(SL_TASK_SYSTEM_ID, SL_SYSTEM_ALIVE_NOTIFY, SL_SYSTEM_ALIVE_NOTIFY_INTERVAL, TIMER_PERIODIC);
	timer_set(SL_TASK_SYSTEM_ID, SL_SYSTEM_STARTUP, SL_SYSTEM_STARTUP_INTERVAL, TIMER_ONE_SHOT);
}

/*---------------------------------------
 * Init state machine for tasks
 * used for app tasks
 ---------------------------------------*/
void app_init_state_machine() {
	
}

/*---------------------------------------------
 * Send first message to trigger start tasks
 * used for app tasks
 ---------------------------------------------*/
void app_task_init() {
#if MPU_SERIAL_INTEFACE_EN
	task_post_pure_msg(SL_TASK_CPU_SERIAL_IF_ID, SL_CPU_SERIAL_IF_INIT);
#endif
}

/*-------------------------------------------------------------------------------------------*/
void TaskPollMPUIf() {
	extern MPU_IncomMsg_t MPU_IncomMsg;
	bool isMsgComming = false;
	uint8_t data;

    while (!isRingBufferCharEmpty(&MPUInterfaceRx)) {
		isMsgComming = true;
        ENTRY_CRITICAL();
        data = ringBufferCharGet(&MPUInterfaceRx);
        EXIT_CRITICAL();
        
		if (MPU_IncomMsg.ind < MAX_MPU_MSG_RECV_LEN) {
			MPU_IncomMsg.buf[MPU_IncomMsg.ind++] = data;
		}
    }
	if (isMsgComming) {
		APP_DBG(TAG, "MPU Message: %s", MPU_IncomMsg.buf);
		if (strcmp((const char*)MPU_IncomMsg.buf, MPU_CORMFIRM) == 0) {
			task_post_pure_msg(SL_TASK_DEVMANAGER_ID, SL_DMANAGER_PROCEDURE_CALL_RESP);
		}
		memset(&MPU_IncomMsg, 0, sizeof(MPU_IncomMsg_t));
	}
}



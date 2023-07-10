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
#if 0
	APP_PRINT(KBLU"\n"
			"    \\              | _)              |   _)             \n"
			"   _ \\   _ \\  _ \\  |  |   _|   _` |   _|  |   _ \\    \\  \n"
			" _/  _\\ .__/ .__/ _| _| \\__| \\__,_| \\__| _| \\___/ _| _| \n"
			"       _|   _|                                          \n"
			);
#endif
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
	MOTOR1_EncoderPinoutInit();
	MOTOR2_EncoderPinoutInit();

#if 0 /* Unit test */
	MOTORS_PWMInit();
	LEDLIFE.initialize(ledLifeInit, ledLifeOn, ledLifeOff);
	EXIT_CRITICAL();
	// MOTOR1_SetPWM(0);
	// MOTOR2_SetPWM(75);
	uint32_t timStamp = millisTick();
	while (1) {
		if (millisTick() - timStamp > 1000) {
			LEDLIFE.Blinking();
			watchdogRst();
			timStamp = millisTick();
		}
	}

#endif

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
	uint8_t data;

    while (!isRingBufferCharEmpty(&MPUInterfaceRx)) {
        ENTRY_CRITICAL();
        data = ringBufferCharGet(&MPUInterfaceRx);
        EXIT_CRITICAL();
		APP_PRINT("[DATA] %d\r\n", data);
        
		if (MPU_IncomMsg.ind < MAX_MPU_MSG_RECV_LEN) {
			MPU_IncomMsg.buf[MPU_IncomMsg.ind++] = data;
		}
		if (strcmp((const char*)MPU_IncomMsg.buf, MPU_CORMFIRM) == 0) {
			task_post_pure_msg(SL_TASK_DEVMANAGER_ID, SL_DMANAGER_PROCEDURE_CALL_RESP);
			memset(&MPU_IncomMsg, 0, sizeof(MPU_IncomMsg_t));
		}
		else if (strcmp((const char*)MPU_IncomMsg.buf, MPU_WEBTIMEOUT) == 0) {
			/* Timeout from web */
			task_post_pure_msg(SL_TASK_DEVMANAGER_ID, SL_DMANAGER_REFRESH_WORKFLOW);
		}
    }
}



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
#include "task_deviceManager.h"
#include "task_console.h"
#include "task_system.h"

#include "io_cfg.h"
#include "sys_cfg.h"
#include "sys_ctl.h"
#include "platform.h"

#include "sys_dbg.h"

#define TAG	"App"

/* Extern variables ----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static void appStartTimer();
static void appInitStateMachine();
static void appTaskInit();

/* Function implementation ---------------------------------------------------*/
int main_app() {
#if 1
	APP_PRINT(KBLU"\r\n"
			"   _____                                 \r\n"
			"  / ___/_________ _____  ____  ___  _____\r\n"
			"  \\__ \\/ ___/ __ `/ __ \\/ __ \\/ _ \\/ ___/\r\n"
			" ___/ / /__/ /_/ / / / / / / /  __/ /    \r\n"
			"/____/\\___/\\__,_/_/ /_/_/ /_/\\___/_/     \r\n"
			"\r\n"
			"VERSION: %s\r\n", FIRMWARE_VERSION
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

	LEDLIFE.initialize(ledLifeInit, ledLifeOn, ledLifeOff);
	LEDFLASH.initialize(ledFlashInit, ledFlashOn, ledFlashOff);
	LEDSTATUS.initialize(ledStatusInit, ledStatusOn, ledStatusOff);
	LEDDIR.initialize(ledDirInit, ledDirOn, ledDirOff);
	LEDFAULT.initialize(ledFaultInit, ledFaultOn, ledFaultOff);
	ENGINES.initialize();

	/* Delay coefficient */
	EEPROM_Read(eepromUSER_SETTING_ADDR, (uint8_t*)&usrAdjust, sizeof(UserSetting_t));
	if (delaySCROLLING_MAGICNUM != usrAdjust.magicNum) {
		usrAdjust.magicNum = delaySCROLLING_MAGICNUM;
		usrAdjust.delayVal = 500;
		EEPROM_Write(eepromUSER_SETTING_ADDR, (uint8_t*)&usrAdjust, sizeof(UserSetting_t));
	}

	/*---------------------------------------------------------------------*/
	/*							 System app setup						   */	
	/*---------------------------------------------------------------------*/
	fatalInit();

	/*---------------------------------------------------------------------*/
	/*						Application task initial					   */	
	/*---------------------------------------------------------------------*/
	appInitStateMachine();
	appStartTimer();
	appTaskInit();

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
void appStartTimer() {
    timerSet(SL_TASK_SYSTEM_ID, SL_SYSTEM_ALIVE_NOTIFY, SL_SYSTEM_ALIVE_NOTIFY_INTERVAL, TIMER_PERIODIC);
}

/*---------------------------------------
 * Init state machine for tasks
 * used for app tasks
 ---------------------------------------*/
void appInitStateMachine() {
}

/*---------------------------------------------
 * Send first message to trigger start tasks
 * used for app tasks
 ---------------------------------------------*/
void appTaskInit() {
	////////////////////////////////////////////
	// TODO: MCU Khởi động xong cho LED Status 
	// sáng, check tất cả cảm biến, nếu có giấy 
	// bên trong máy sẵn, cho nhả ra
	////////////////////////////////////////////
	taskPostPureMsg(SL_TASK_DEVMANAGER_ID, SL_DMANAGER_CONVEYOR_CLEANUP);
}

/*-------------------------------------------------------------------------------------------*/
void TaskPollMPUIf() {
	uint8_t u8Data;
	bool execCompleted = false;

    while (!isRingBufferCharEmpty(&MPUInterfaceRx)) {
        ENTRY_CRITICAL();
        u8Data = ringBufferCharGet(&MPUInterfaceRx);
        EXIT_CRITICAL();

		if (MPU_IncomMsg.ind < MAX_MPU_MSG_RECV_LEN) {
			MPU_IncomMsg.buf[MPU_IncomMsg.ind++] = u8Data;
		}
		else {
			memset(&MPU_IncomMsg, 0, sizeof(MPU_IncomMsg_t));
		}
		
		if (strcmp((const char*)MPU_IncomMsg.buf, MPU_CORMFIRM) == 0) {
			APP_PRINT("MPU_CORMFIRM\r\n");
			taskPostPureMsg(SL_TASK_DEVMANAGER_ID, SL_DMANAGER_PROCEDURE_CALL_RESP);
			execCompleted = true;
		}
		else if (strcmp((const char*)MPU_IncomMsg.buf, MPU_WEBTIMEOUT) == 0) {
			APP_PRINT("MPU_WEBTIMEOUT\r\n");
			taskPostPureMsg(SL_TASK_DEVMANAGER_ID, SL_DMANAGER_ENTRY_IDLING);
			execCompleted = true;
		}
		else if (strcmp((const char*)MPU_IncomMsg.buf, MPU_NOTIFYLOGIN) == 0) {
			APP_PRINT("MPU_NOTIFYLOGIN\r\n");
			taskPostPureMsg(SL_TASK_DEVMANAGER_ID, SL_DMANAGER_START_WORKFLOW_REQ);
			execCompleted = true;
		}

		if (execCompleted) {
			memset(&MPU_IncomMsg, 0, sizeof(MPU_IncomMsg_t));
		}
    }
}



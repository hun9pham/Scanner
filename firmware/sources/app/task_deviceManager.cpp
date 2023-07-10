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
#include "task_deviceManager.h"

#include "platform.h"
#include "io_cfg.h"
#include "sys_cfg.h"
#include "sys_ctl.h"

#include "sys_dbg.h"

#define TAG	"TaskDeviceManager"

/* Extern variables ----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static uint32_t now = 0;

/* Private function prototypes -----------------------------------------------*/
static void MOTORS_Monitors();
static void sendPCREQandWait(const char *pcMsg) {
	task_post_common_msg(SL_TASK_DEVMANAGER_ID, SL_DMANAGER_PROCEDURE_CALL_REQ,
			(uint8_t*)pcMsg, strlen(pcMsg));
	APP_DBG(TAG, "Waiting....");
}

static uint8_t assertSensState(uint8_t sensId) {
	uint8_t ret = 0;
	uint8_t (*funcPointer)();

	if (sensId == 1) {
		funcPointer = readInput1;
	}
	else if (sensId == 2) {
		funcPointer = readInput2;
	}
	else if (sensId == 3) {
		funcPointer = readInput3;
	}
	else return 0;

	uint32_t timeStamp = millisTick();
	while (millisTick() - timeStamp < 150) {
		ret = funcPointer();
	}
	return ret;
}

static void workFlowSetTimeOut() {
	timer_set(SL_TASK_DEVMANAGER_ID, SL_DMANAGER_REFRESH_WORKFLOW, 10000, TIMER_ONE_SHOT);
}

static void workFlorClrTimeOut() {
	timer_remove_attr(SL_TASK_DEVMANAGER_ID, SL_DMANAGER_REFRESH_WORKFLOW);
}

/* Function implementation ---------------------------------------------------*/
void TaskDeviceManager(ak_msg_t* msg) {
	switch (msg->sig) {
	case SL_DMANAGER_REFRESH_WORKFLOW: {
		APP_DBG_SIG(TAG, "SL_DMANAGER_REFRESH_WORKFLOW");
		/* Check tất cả cảm biến, nếu có giấy bên trong máy sẵn, cho nhả ra */

		/* Giấy đã được nhả ra hết */
		if (assertSensState(1) != 0 && assertSensState(2) != 0 && assertSensState(3) != 0) {
			LEDFLASH.OffState();
			LEDDIR.OffState();
			LEDFAULT.OffState();
			ENGINES.setMOTORS(DUOMOTORS, STOPPING);
			// makeBeepSound(50);
			task_polling_set_ability(SL_TAKS_POLL_DEVMANAGER_ID, AK_ENABLE);
			memset(&MPU_IncomMsg, 0, sizeof(MPU_IncomMsg_t));
			devStagePolling = STAGE_1ST;
			break;
		}

		/* Lỗi giấy còn trong máy */
		APP_DBG(TAG, "Conveyor detects pappers");
		if (assertSensState(1) == 0 || assertSensState(2) == 0) {
			ENGINES.setMOTORS(MOTOR_FRONT, SCROLLING);
		}
		if (assertSensState(3) == 0) {
			ENGINES.setMOTORS(MOTOR_REAR, SCROLLING);
		}
		timer_set(SL_TASK_DEVMANAGER_ID, SL_DMANAGER_REFRESH_WORKFLOW, 350, TIMER_ONE_SHOT);
	}
	break;

	case SL_DMANAGER_PROCEDURE_CALL_REQ: {
		APP_DBG_SIG(TAG, "SL_DMANAGER_PROCEDURE_CALL_REQ");

		const char *pcrqMsg = (const char*)get_data_common_msg(msg);
		APP_DBG(TAG, "Procedure Call - %s", pcrqMsg);
		putMPUMessage(pcrqMsg);

		/* Wait for MPU Response to continue */
		task_polling_set_ability(SL_TAKS_POLL_DEVMANAGER_ID, AK_DISABLE);
	}
	break;

	case SL_DMANAGER_PROCEDURE_CALL_RESP: {
		APP_DBG_SIG(TAG, "SL_DMANAGER_PROCEDURE_CALL_RESP");

		/* Continue workflow */
		task_polling_set_ability(SL_TAKS_POLL_DEVMANAGER_ID, AK_ENABLE);
	}
	break;

	case SL_DMANAGER_EXCEPTION_SHORTPAPER: {
		APP_DBG_SIG(TAG, "SL_DMANAGER_EXCEPTION_SHORTPAPER");

	}
	break;

	case SL_DMANAGER_EXCEPTION_PAPERJAM: {
		APP_DBG_SIG(TAG, "SL_DMANAGER_EXCEPTION_PAPERJAM");

		// makeBeepSound(100);
		/* LED Fault sáng */
		LEDFAULT.OnState();
		/* Gửi message “#3\r\n” */
		const char *repMsg = MPU_PAPERJAM;
		putMPUMessage(repMsg);
		devStagePolling = STAGE_IDLE;
	}
	break;

	default:
	break;
	}
}

void TaskPollDevManager() {
	if (millisTick() - now > 300) {
		now = millisTick();

		switch (devStagePolling) {
		case STAGE_IDLE:
		break;

		case STAGE_1ST: { /* Chờ cảm biến 1 phát hiện giấy đưa vào */
			/* LED Dir blinking */
			LEDDIR.Blinking();
			/* Check cảm biến 1 */
			if (assertSensState(1) == 0) {
				APP_DBG(TAG, "[DONE-STAGE] STAGE_1ST");
				LEDDIR.OffState();
				ENGINES.setMOTORS(MOTOR_FRONT, SCROLLING);
				/* Next stage */
				devStagePolling = STAGE_2ND;
				workFlowSetTimeOut();
			}
		}
		break;

    	case STAGE_2ND: { /* Cuốn vào đến khi cảm biến 3 phát hiện */
			/* 
				Exception: Giấy ngắn (cảm biến 1 phát hiện giấy cuốn vào, 
				cảm biến 1 phát hiện hết giấy, cảm biến 2 phát hiện có 
				giấy, cảm biến 3 ko có giấy) 
			*/
#if 0
			if ((assertSensState(1) == 1 && assertSensState(2) == 0) &&
				assertSensState(3) == 1)
			{
				/* 
					REQ: Giấy ngắn, báo chụp ảnh, đợi confirm rồi nhả giấy ra
				*/
				/* Dừng động cơ */
				ENGINES.setMotorFront(ENGINE_STANDSTILL);
				/* Gửi message yêu cầu chụp hình */
				sendPCREQandWait(MPU_REQSCREENSHOT);
				/* Exception case handle */
				devStagePolling = EXCEPTION_CASE;
			}
			else if ((assertSensState(1) == 0 && assertSensState(2) == 0) &&
				assertSensState(3) == 0)
			{
				APP_DBG(TAG, "[DONE-STAGE] STAGE_2ND");
				/* Dừng động cơ */
				ENGINES.setMotorFront(ENGINE_STANDSTILL);
				/* On LED Flash */
				LEDFLASH.OnState();
				/* Gửi message yêu cầu chụp hình */
				sendPCREQandWait(MPU_REQSCREENSHOT);
				/* Next stage */
				devStagePolling = STAGE_3RD;
			}
#else
			if (assertSensState(3) == 0) {
				APP_DBG(TAG, "[DONE-STAGE] STAGE_2ND");
				/* Dừng động cơ */
				ENGINES.setMOTORS(MOTOR_FRONT, STOPPING);
				/* On LED Flash */
				LEDFLASH.OnState();
				/* Gửi message yêu cầu chụp hình */
				sendPCREQandWait(MPU_REQSCREENSHOT);
				/* Next stage */
				devStagePolling = STAGE_3RD;
				workFlowSetTimeOut();
			}
#endif
		}
		break;

    	case STAGE_3RD: { /* Cảm biến 1 ko thấy giấy, cảm biến 2 ko thấy giấy */
			if (ENGINES.readMOTORStateCtl(MOTOR_FRONT) == STOPPING && 
				ENGINES.readMOTORStateCtl(MOTOR_REAR) == STOPPING)
			{
				/* Chạy hai motor để tiếp tục cuộn giấy ra */
				ENGINES.setMOTORS(DUOMOTORS, SCROLLING);
			}
			if (assertSensState(1) != 0 && assertSensState(2) != 0) { /* Giấy đã đi qua hết sensors[1:2] */
				APP_DBG(TAG, "[DONE-STAGE] STAGE_3RD");
				/* Dừng motor front và tiếp tục chạy motor rear để tránh quá tải */
				ENGINES.setMOTORS(MOTOR_FRONT, STOPPING);
				/* Gửi message yêu cầu chụp hình */
				sendPCREQandWait(MPU_REQSCREENSHOT);
				/* Next stage */
				devStagePolling = STAGE_4TH;
				workFlowSetTimeOut();
			}
		}
		break;

    	case STAGE_4TH: { /* Chờ MPU Confirm rồi gửi command hết giấy tới MPU */
			APP_DBG(TAG, "[DONE-STAGE] STAGE_4TH");
			/* Gửi command hết giấy mà không cần chờ phản hồi */
			const char *notifyMsg = MPU_OUTOFPAPPER;
			putMPUMessage(notifyMsg);
			APP_DBG(TAG, "Command - %s", notifyMsg);
			/* Next stage */
			devStagePolling = STAGE_5TH;
			workFlorClrTimeOut();
		}
		break;

    	case STAGE_5TH: { /* Sau khi gửi message hết giấy thì nhả giấy ra khỏi bộ cuốn giấy */
			if (assertSensState(3) != 0) { /* Giấy đã được cuộn ra hết */
				APP_DBG(TAG, "[DONE-STAGE] STAGE_5TH");
				delayMillis(300); /* Delay them 1 chút để giấy cuốn hết ra ngoài */
				ENGINES.setMOTORS(DUOMOTORS, STOPPING);
				/* Tắt LED Flash */
				LEDFLASH.OffState();
				/* Reset stage */
				devStagePolling = STAGE_1ST;
			}
		}
		break;

		case EXCEPTION_CASE: {
			task_post_pure_msg(SL_TASK_DEVMANAGER_ID, SL_DMANAGER_REFRESH_WORKFLOW);
		}
		break;
		
		default:
		break;
		}

		/*
			-TODO: Check motors encoder if is running
			Kẹt giấy: động cơ được cho quay nhưng encoder đọc ko thấy quay, 
			dừng động cơ báo lỗi fault. Led fault sáng lên gửi message “#3\r\n”.
		*/
		MOTORS_Monitors();
	}
}

void MOTORS_Monitors() {
	if (ENGINES.readMOTORStateCtl(MOTOR_FRONT) != STOPPING &&
		ENGINES.readMOTORStateCtl(MOTOR_REAR) != STOPPING)
	{
		uint8_t isRun = 0;
		isRun += ENGINES.isRun(MOTOR_FRONT);
		isRun += ENGINES.isRun(MOTOR_REAR);
		if (isRun != (uint8_t)(true + true)) {
			task_post_pure_msg(SL_TASK_DEVMANAGER_ID, SL_DMANAGER_EXCEPTION_PAPERJAM);
		}
		else {
			/* LED Fault tắt */
			LEDFAULT.OffState();
		}
	}
	// else if (ENGINES.readMOTORStateCtl(MOTOR_FRONT) != STOPPING) {
	// 	uint8_t isRun = 0;
	// 	isRun += ENGINES.isRun(MOTOR_FRONT);
	// 	if (!isRun) {
	// 		task_post_pure_msg(SL_TASK_DEVMANAGER_ID, SL_DMANAGER_EXCEPTION_PAPERJAM);
	// 	}
	// 	else {
	// 		/* LED Fault tắt */
	// 		LEDFAULT.OffState();
	// 	}
	// }
	// else if (ENGINES.readMOTORStateCtl(MOTOR_REAR) != STOPPING) {
	// 	uint8_t isRun = 0;
	// 	isRun += ENGINES.isRun(MOTOR_REAR);
	// 	if (!isRun) {
	// 		task_post_pure_msg(SL_TASK_DEVMANAGER_ID, SL_DMANAGER_EXCEPTION_PAPERJAM);
	// 	}
	// 	else {
	// 		/* LED Fault tắt */
	// 		LEDFAULT.OffState();
	// 	}
	// }
}
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

/* Function implementation ---------------------------------------------------*/
void TaskDeviceManager(ak_msg_t* msg) {
	switch (msg->sig) {
	case SL_DMANAGER_REFRESH_WORKFLOW: {
		APP_DBG_SIG(TAG, "SL_DMANAGER_REFRESH_WORKFLOW");
		/* Check tất cả cảm biến, nếu có giấy bên trong máy sẵn, cho nhả ra */

		/* Giấy đã được nhả ra hết */
		if (assertSensState(1) != 0 && assertSensState(2) != 0 && assertSensState(3) != 0) {
			/* Turn off all motors */
			ENGINES.setMOTORS(DUOMOTORS, STOPPING);
			/* Tắt all LEDs */
			LEDFLASH.OffState();
			LEDDIR.OffState();
			LEDFAULT.OffState();
			devStagePolling = STAGE_1ST;
			break;
		}

		/* Lỗi giấy còn trong máy */
		APP_DBG(TAG, "Conveyor detects papers");
		if (assertSensState(1) == 0 || assertSensState(2) == 0) {
			ENGINES.setMOTORS(MOTOR_FRONT, SCROLL_BACKWARD);
		}
		if (assertSensState(3) == 0) {
			ENGINES.setMOTORS(MOTOR_REAR, SCROLL_BACKWARD);
		}
		timer_set(SL_TASK_DEVMANAGER_ID, SL_DMANAGER_REFRESH_WORKFLOW, 350, TIMER_ONE_SHOT);
	}
	break;

	case SL_DMANAGER_PROCEDURE_CALL_REQ: {
		APP_DBG_SIG(TAG, "SL_DMANAGER_PROCEDURE_CALL_REQ");

		const char *pcrqMsg = (const char*)get_data_common_msg(msg);
		APP_DBG(TAG, "Procedure Call - %s, wait ...", pcrqMsg);
		putMPUMessage(pcrqMsg);

		/* Wait for MPU Response to continue */
		task_polling_set_ability(SL_TASK_POLL_DEVMANAGER_ID, AK_DISABLE);
	}
	break;

	case SL_DMANAGER_PROCEDURE_CALL_RESP: {
		APP_DBG_SIG(TAG, "SL_DMANAGER_PROCEDURE_CALL_RESP");

		/* Continue workflow */
		task_polling_set_ability(SL_TASK_POLL_DEVMANAGER_ID, AK_ENABLE);
	}
	break;

	case SL_DMANAGER_EXCEPTION_SHORTPAPER: {
		APP_DBG_SIG(TAG, "SL_DMANAGER_EXCEPTION_SHORTPAPER");

	}
	break;

	case SL_DMANAGER_EXCEPTION_PAPERJAM: {
		APP_DBG_SIG(TAG, "SL_DMANAGER_EXCEPTION_PAPERJAM");

		makeBeepSound(100);
		/* Đưng động cơ */
		ENGINES.setMOTORS(DUOMOTORS, STOPPING);
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
				ENGINES.setMOTORS(MOTOR_FRONT, SCROLL_FORDWARD);
				if (assertSensState(2) == 0) {
					APP_PRINT("[DONE-STAGE] STAGE_1ST\r\n");
					LEDDIR.OffState();
					makeBeepSound(100);
					/* Next stage */
					devStagePolling = STAGE_2ND;
					SENSOR2_OutOfPaper = false;
					SENSOR1_DectecShortPaper = false;
					enableEXTI(&EXTI6_EnableDetectFirstStopScroll);
					enableEXTI(&EXTI4_EnableDetectShortPaper);
				}
			}
			else {
				ENGINES.setMOTORS(MOTOR_FRONT, STOPPING);
			}
		}
		break;

    	case STAGE_2ND: { /* Cuốn vào đến khi cảm biến 3 phát hiện */
			/* 
				Exception: Giấy ngắn (cảm biến 1 phát hiện giấy cuốn vào, 
				cảm biến 1 phát hiện hết giấy, cảm biến 2 phát hiện có 
				giấy, cảm biến 3 ko có giấy) 
			*/
			if (SENSOR1_DectecShortPaper) {
				/* 
					REQ: Giấy ngắn, báo chụp ảnh, đợi confirm rồi nhả giấy ra
				*/
				/* On LED Flash */
				LEDFLASH.OnState();
				/* Dừng động cơ */
				ENGINES.setMOTORS(MOTOR_FRONT, STOPPING);
				delayMillis(100);
				/* Gửi message yêu cầu chụp hình */
				sendPCREQandWait(MPU_REQSCREENSHOT);
				/* Exception case handle */
				devStagePolling = EXCEPTION_CASE;
				APP_PRINT("goto EXCEPTION_CASE\r\n");
			}
			else if (assertSensState(3) == 0) {
				APP_PRINT("[DONE-STAGE] STAGE_2ND\r\n");
				/* Dừng động cơ */
				ENGINES.setMOTORS(MOTOR_FRONT, STOPPING);
				delayMillis(100);
				/* On LED Flash */
				LEDFLASH.OnState();
				/* Gửi message yêu cầu chụp hình */
				sendPCREQandWait(MPU_REQSCREENSHOT);
				/* Next stage */
				devStagePolling = STAGE_3RD;
				enableEXTI(&EXTI5_EnableDetectOutOfPaper);
				disableEXTI(&EXTI4_EnableDetectShortPaper);
			}
		}
		break;

    	case STAGE_3RD: { /* Cảm biến 1 ko thấy giấy, cảm biến 2 ko thấy giấy */
			if (SENSOR2_OutOfPaper == false) {
				/* Chạy hai motor để tiếp tục cuộn giấy ra */
				ENGINES.setMOTORS(DUOMOTORS, SCROLL_FORDWARD);
				delayMillis(usrAdjust.delayVal);
				ENGINES.setMOTORS(DUOMOTORS, STOPPING);
				delayMillis(100);
				/* Gửi message yêu cầu chụp hình */
				sendPCREQandWait(MPU_REQSCREENSHOT);
			}
			else { /* Giấy đã đi qua hết sensors[1:2] */
				APP_PRINT("[DONE-STAGE] STAGE_3RD\r\n");
				/* Dừng motor front và tiếp tục chạy motor rear để tránh quá tải */
				ENGINES.setMOTORS(DUOMOTORS, STOPPING);
				/* Next stage */
				devStagePolling = STAGE_4TH;
			}
			/* On LED Flash */
			LEDFLASH.OnState();
		}
		break;

    	case STAGE_4TH: { /* Chờ MPU Confirm rồi gửi command hết giấy tới MPU */
			APP_PRINT("[DONE-STAGE] STAGE_4TH\r\n");
			/* Gửi command hết giấy mà không cần chờ phản hồi */
			const char *notifyMsg = MPU_OUTOFPAPPER;
			putMPUMessage(notifyMsg);
			APP_DBG(TAG, "Command - %s", notifyMsg);
			/* Next stage */
			devStagePolling = STAGE_5TH;
		}
		break;

    	case STAGE_5TH: { /* Sau khi gửi message hết giấy thì nhả giấy ra khỏi bộ cuốn giấy */
			ENGINES.setMOTORS(DUOMOTORS, SCROLL_BACKWARD);
			if (assertSensState(2) != 0) { /* Giấy đã được cuộn ra hết */
				APP_PRINT("[DONE-STAGE] STAGE_5TH\r\n");
				delayMillis(500); /* Delay them 1 chút để giấy cuốn hết ra ngoài */
				ENGINES.setMOTORS(DUOMOTORS, STOPPING);
				/* Tắt LED Flash */
				LEDFLASH.OffState();
				/* Reset stage */
				timer_set(SL_TASK_SYSTEM_ID, SL_SYSTEM_ENTRY_IDLING, 350, TIMER_ONE_SHOT);
			}
		}
		break;

		case EXCEPTION_CASE: {
			/* Gửi command hết giấy mà không cần chờ phản hồi */
			const char *notifyMsg = MPU_OUTOFPAPPER;
			putMPUMessage(notifyMsg);
			task_post_pure_msg(SL_TASK_DEVMANAGER_ID, SL_DMANAGER_REFRESH_WORKFLOW);
			timer_set(SL_TASK_SYSTEM_ID, SL_SYSTEM_ENTRY_IDLING, 350, TIMER_ONE_SHOT);
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
	bool isFault = false;
	bool isMOTORRun;
	static uint8_t assertPaperJameCnt = 0;
	static uint32_t timDelay = millisTick();

	if (millisTick() - timDelay > 1000) {
		timDelay = millisTick();

		if (ENGINES.readMOTORStateCtl(MOTOR_FRONT) != STOPPING) {
			isMOTORRun = ENGINES.isRun(MOTOR_FRONT);
			if (!isMOTORRun) {
				APP_PRINT("MOTOR_FRONT = 0\r\n");
				isFault = true;
			}
			else {
				APP_PRINT("MOTOR_FRONT = 1\r\n");
			}
		}
		if (ENGINES.readMOTORStateCtl(MOTOR_REAR) != STOPPING) {
			isMOTORRun = ENGINES.isRun(MOTOR_REAR);
			if (!isMOTORRun) {
				APP_PRINT("MOTOR_REAR = %d\r\n", ENGINES.isRun(MOTOR_REAR));
				isFault = true;
			}
		}
		if (isFault) {
			++assertPaperJameCnt;
			if (assertPaperJameCnt == 3) { /* Trong 3s, nếu động cơ vẫn kẹt thì report */
				task_post_pure_msg(SL_TASK_DEVMANAGER_ID, SL_DMANAGER_EXCEPTION_PAPERJAM);
			}
		}
		else {
			assertPaperJameCnt = 0;
			/* LED Fault tắt */
			if (LEDFAULT.getState() == INDICATOR_BRIGHT) {
				LEDFAULT.OffState();
			}
		}
	}
}
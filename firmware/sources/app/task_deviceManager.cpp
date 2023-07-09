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
static void sendMsgREQ(const char *pcMsg) {
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
	while (millisTick() - timeStamp > 150) {
		ret = funcPointer();
	}
	return ret;
}

/* Function implementation ---------------------------------------------------*/
void TaskDeviceManager(ak_msg_t* msg) {
	switch (msg->sig) {
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

	case SL_DMANAGER_PROCEDURE_CALL_RESP_TO: {
		APP_DBG_SIG(TAG, "SL_DMANAGER_PROCEDURE_CALL_RESP_TO");

	}
	break;

	case SL_DMANAGER_CHECK_PAPER_EXISTENCE: {
		APP_DBG_SIG(TAG, "SL_DMANAGER_CHECK_PAPER_EXISTENCE");

		/* Giấy đã được nhả ra hết */
		if (assertSensState(1) != 0 && assertSensState(2) != 0 && assertSensState(3) != 0) {
			timer_remove_attr(SL_TASK_DEVMANAGER_ID, SL_DMANAGER_CHECK_PAPER_EXISTENCE);
			ENGINES.setOperation(ENGINE_STANDSTILL);
			task_polling_set_ability(SL_TAKS_POLL_DEVMANAGER_ID, AK_ENABLE);
			break;
		}

		/* Lỗi giấy còn trong máy */
		APP_DBG(TAG, "Conveyor detects pappers");
		if (assertSensState(1) == 0 || assertSensState(2) == 0) {
			ENGINES.setMotorFront(ENGINE_BACKWARD);
		}
		if (assertSensState(3) == 0) {
			ENGINES.setMotorRear(ENGINE_BACKWARD);
		}
		timer_set(SL_TASK_DEVMANAGER_ID, SL_DMANAGER_CHECK_PAPER_EXISTENCE, 
			SL_DMANAGER_CHECK_PAPER_EXISTENCE_INTERVAL, TIMER_PERIODIC);
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
				APP_DBG(TAG, "[ON-STAGE] STAGE_1ST");
				LEDDIR.OffState();
				ENGINES.setMotorFront(ENGINE_BACKWARD);
				/* Next stage */
				devStagePolling = STAGE_2ND;
			}
		}
		break;

    	case STAGE_2ND: { /* Cuốn vào đến khi cảm biến 3 phát hiện */
			if (assertSensState(3) == 0) {
				APP_DBG(TAG, "[ON-STAGE] STAGE_2ND");
				/* Dừng động cơ */
				ENGINES.setMotorFront(ENGINE_STANDSTILL);
				/* On LED Flash */
				LEDFLASH.OnState();
				/* Gửi message yêu cầu chụp hình */
				sendMsgREQ(MPU_REQSCREENSHOT);
				/* Next stage */
				devStagePolling = STAGE_3RD;
			}
		}
		break;

    	case STAGE_3RD: { /* Cảm biến 1 ko thấy giấy, cảm biến 2 ko thấy giấy */
			if (ENGINES.getMotorFront() == ENGINE_STANDSTILL && ENGINES.getMotorRear() == ENGINE_STANDSTILL) {
				/* Chạy hai motor để tiếp tục cuộn giấy ra */
				ENGINES.setOperation(ENGINE_BACKWARD);
			}
			if (assertSensState(1) != 0 && assertSensState(2) != 0) { /* Giấy đã đi qua hết sensors[1:2] */
				APP_DBG(TAG, "[ON-STAGE] STAGE_3RD");
				/* Dừng motor front và tiếp tục chạy motor rear để tránh quá tải */
				ENGINES.setMotorFront(ENGINE_STANDSTILL);
				/* Gửi message yêu cầu chụp hình */
				sendMsgREQ(MPU_REQSCREENSHOT);
				/* Next stage */
				devStagePolling = STAGE_4TH;
			}
		}
		break;

    	case STAGE_4TH: { /* Chờ MPU Confirm rồi gửi command hết giấy tới MPU */
			APP_DBG(TAG, "[ON-STAGE] STAGE_4TH");
			/* Gửi command hết giấy mà không cần chờ phản hồi */
			const char *notifyMsg = MPU_OUTOFPAPPER;
			putMPUMessage(notifyMsg);
			APP_DBG(TAG, "Command - %s", notifyMsg);
			/* Next stage */
			devStagePolling = STAGE_5TH;
		}
		break;

    	case STAGE_5TH: { /* Sau khi gửi message hết giấy thì nhả giấy ra khỏi bộ cuốn giấy */
			APP_DBG(TAG, "[ON-STAGE] STAGE_5TH");

			if (assertSensState(3) != 0) { /* Giấy đã được cuộn ra hết */
				delayMillis(300); /* Delay them 1 chút để giấy cuốn hết ra ngoài */
				ENGINES.setOperation(ENGINE_STANDSTILL);
				/* Tắt LED Flash */
				LEDFLASH.OffState();
				/* Reset stage */
				devStagePolling = STAGE_1ST;
			}
		}
		break;
		
		default:
		break;
		}
	}
}

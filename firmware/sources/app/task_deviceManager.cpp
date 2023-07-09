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

static void motorsCtlREQ(uint8_t ctl) {
	task_post_common_msg(SL_TASK_DEVMANAGER_ID, SL_DMANAGER_CONTROL_MOTORS,
							&ctl, sizeof(uint8_t));
}

/* Function implementation ---------------------------------------------------*/
void TaskDeviceManager(ak_msg_t* msg) {
	switch (msg->sig) {
	case SL_DMANAGER_CONTROL_MOTORS: {
		APP_DBG_SIG(TAG, "SL_DMANAGER_CONTROL_MOTORS");

		uint8_t ctl = *(get_data_common_msg(msg));
		APP_DBG(TAG, "Control motors - %s", ctl == ENGINE_BACKWARD ? "BACKWARD" : 
					ctl == ENGINE_FORWARD ? "FORWARD" : "STANDSTILL");
		ENGINES.setOperation(ctl);
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

		task_polling_set_ability(SL_TAKS_POLL_DEVMANAGER_ID, AK_ENABLE);
	}
	break;

	case SL_DMANAGER_PROCEDURE_CALL_RESP_TO: {
		APP_DBG_SIG(TAG, "SL_DMANAGER_PROCEDURE_CALL_RESP_TO");

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
			if (readSensor1() == 0) {
				LEDDIR.OffState();
				motorsCtlREQ(ENGINE_BACKWARD);
				/* Next stage */
				devStagePolling = STAGE_2ND;
			}
		}
		break;

    	case STAGE_2ND: { /* Cuốn vào đến khi cảm biến 3 phát hiện */
			if (readSensor3() == 0) {
				/* Dừng động cơ */
				motorsCtlREQ(ENGINE_STANDSTILL);
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
			if (readSensor1() != 0 && readSensor2() != 0) {
				/* Gửi message yêu cầu chụp hình */
				sendMsgREQ(MPU_REQSCREENSHOT);
				/* Next stage */
				devStagePolling = STAGE_4TH;
			}
		}
		break;

    	case STAGE_4TH: { /* Chờ MPU Confirm rồi gửi command hết giấy tới MPU */
			/* Gửi command hết giấy mà không cần chờ phản hồi */
			const char *notifyMsg = MPU_OUTOFPAPPER;
			putMPUMessage(notifyMsg);
			APP_DBG(TAG, "Command - %s", notifyMsg);
			/* Next stage */
			devStagePolling = STAGE_5TH;
		}
		break;

    	case STAGE_5TH: { /* Sau khi gửi message hết giấy thì nhả giấy ra khỏi bộ cuốn giấy */
			ENGINES.setOperation(ENGINE_BACKWARD);
			while (readSensor3() == 0) {

			}
			delayMillis(500);
			ENGINES.setOperation(ENGINE_STANDSTILL);
			/* Tắt LED Flash */
			LEDFLASH.OffState();
			/* Reset stage */
			devStagePolling = STAGE_1ST;
		}
		break;
		
		default:
		break;
		}
	}
}

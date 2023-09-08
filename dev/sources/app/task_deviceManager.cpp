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

#define TAG	"TaskDevManager"

#define USR_DBG_STAGE_RUN_EN	(1)

/* Extern variables ----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static DevMngCtlStage_t scannerStageRun = STAGE_IDLE;
#if USR_DBG_STAGE_RUN_EN
static DevMngCtlStage_t dbgStageRun = STAGE_EXCP;
#endif

static uint32_t timBlinky;

/* Private function prototypes -----------------------------------------------*/
static void MOTORS_Monitoring();
static void sendPCREQandWait(const char *pcMsg) {
	taskPostCommonMsg(SL_TASK_DEVMANAGER_ID, SL_DMANAGER_PROCEDURE_CALL_REQ,
						(uint8_t*)pcMsg, strlen(pcMsg));
	delayMillis(250);
}

static bool customDelay(uint32_t timeOut, uint32_t timStamp) {
	////////////////////////////////////
	// TODO: Cuộn hết giấy đến khi cảm
	// biến 1 không thấy giấy nữa
	///////////////////////////////////
	if (readMidSensor() == PAPER_UNDETECTED) {
		return true;
	}

	////////////////////////////////////
	// TODO: Cuộn hết giấy đến khi hết
	// thời gian chờ.
	///////////////////////////////////
	if (millisTick() - timStamp > timeOut) {
		return true;
	}

	return false;
}

/* Function implementation ---------------------------------------------------*/
void TaskDeviceManager(ak_msg_t* msg) {
	switch (msg->sig) {
	case SL_DMANAGER_CONVEYOR_CLEANUP: {
		APP_DBG_SIG(TAG, "SL_DMANAGER_CONVEYOR_CLEANUP");
		//////////////////////////////////////////////
		// TODO: Check tất cả cảm biến, nếu có giấy
		// bên trong máy sẵn, cho nhả ra
		//////////////////////////////////////////////

		/* Giấy đã được nhả ra hết */
		if (readFrontSensor() == PAPER_UNDETECTED 
			&& readMidSensor() == PAPER_UNDETECTED 
			&& readRearSensor() == PAPER_UNDETECTED)
		{
			APP_DBG(TAG, "PAPERs hasn't detected on CONVEYOR -> SCROLLING");
			taskPostPureMsg(SL_TASK_DEVMANAGER_ID, SL_DMANAGER_ENTRY_IDLING);
			break;
		}

		/* Lỗi giấy còn trong máy -> Cho chạy động cơ để cuộn hết giấy ra ngoài */
		APP_DBG(TAG, "PAPERs has detected on CONVEYOR -> SCROLLING");
		if (ENGINES.readMOTORStateCtl(MOTOR_FRONT) != SCROLL_BACKWARD) {
			ENGINES.setMOTORS(MOTOR_FRONT, SCROLL_BACKWARD);
		}
		if (ENGINES.readMOTORStateCtl(MOTOR_REAR) != SCROLL_BACKWARD) {
			ENGINES.setMOTORS(MOTOR_REAR, SCROLL_BACKWARD);
		}
		timerSet(SL_TASK_DEVMANAGER_ID, SL_DMANAGER_CONVEYOR_CLEANUP, 350, TIMER_ONE_SHOT);
	}
	break;

	case SL_DMANAGER_ENTRY_IDLING: {
		APP_DBG_SIG(TAG, "SL_DMANAGER_ENTRY_IDLING");
		
		/*
			MCU khởi động xong cho led status sáng
		*/
		LEDSTATUS.OnState();
		LEDFLASH.OffState();
		LEDDIR.OffState();
		LEDFAULT.OffState();
		ENGINES.setMOTORS(DUOMOTORS, STOPPING);
		EXTI_DetectOutOfPaper(false);
		EXTI_DetectEndpointPaper(false);
		EXTI_DetectShortPaper(false);
		taskPollingSetAbility(SL_TASK_POLL_DEVMANAGER_ID, AK_DISABLE);
	}
	break;

	case SL_DMANAGER_START_WORKFLOW_REQ: {
		APP_DBG_SIG(TAG, "SL_DMANAGER_START_WORKFLOW_REQ");

		scannerStageRun = STAGE_1ST;
		timBlinky = 0;
		taskPollingSetAbility(SL_TASK_POLL_DEVMANAGER_ID, AK_ENABLE);
	}
	break;

	case SL_DMANAGER_PROCEDURE_CALL_REQ: {
		APP_DBG_SIG(TAG, "SL_DMANAGER_PROCEDURE_CALL_REQ");

		const char *pcrqMsg = (const char*)getDataCommonMsg(msg);
		APP_DBG(TAG, "Procedure Call - %s, wait ...", pcrqMsg);
		putMPUMessage(pcrqMsg);

		/* Wait for MPU Response to continue */
		taskPollingSetAbility(SL_TASK_POLL_DEVMANAGER_ID, AK_DISABLE);
	}
	break;

	case SL_DMANAGER_PROCEDURE_CALL_RESP: {
		APP_DBG_SIG(TAG, "SL_DMANAGER_PROCEDURE_CALL_RESP");

		/* Continue workflow */
		taskPollingSetAbility(SL_TASK_POLL_DEVMANAGER_ID, AK_ENABLE);
	}
	break;

	case SL_DMANAGER_HANDLE_EXCP_PAPER_JAM: {
		APP_DBG_SIG(TAG, "SL_DMANAGER_HANDLE_EXCP_PAPER_JAM");

		makeBeepSound(100);
		/* Đưng động cơ */
		ENGINES.setMOTORS(DUOMOTORS, STOPPING);
		/* LED Fault sáng */
		LEDFAULT.OnState();
		/* Gửi message “#3\r\n” */
		const char *repMsg = MPU_PAPERJAM;
		putMPUMessage(repMsg);
		scannerStageRun = STAGE_IDLE;
	}
	break;

	default:
	break;
	}
}

void TaskPollDevManager() {
#if USR_DBG_STAGE_RUN_EN
	if (dbgStageRun != scannerStageRun) {
		dbgStageRun = scannerStageRun;
		APP_DBG(TAG, "[RUNNING] STAGE_%d\r\n", dbgStageRun);
	}
#endif
	switch (scannerStageRun) {
	case STAGE_IDLE: {

	}
	break;

	case STAGE_1ST: {
		/////////////////////////////////////////////////
		// TODO: Chờ cảm biến 1 phát hiện giấy đưa vào
		/////////////////////////////////////////////////
		if (millisTick() - timBlinky > 250) {
			LEDDIR.Blinking();
			timBlinky = millisTick();
		}

		if (readFrontSensor() == PAPER_DETECTED) {
			ENGINES.setMOTORS(MOTOR_FRONT, SCROLL_FORDWARD);
			if (readMidSensor() == PAPER_DETECTED) {
				LEDDIR.OffState();
				makeBeepSound(100);
				scannerStageRun = STAGE_2ND;
				EXTI_DetectEndpointPaper(true);
				EXTI_DetectShortPaper(true);
				isPaperShortDetected = false;
			}
		}
		else {
			ENGINES.setMOTORS(MOTOR_FRONT, STOPPING);
		}
	}
	break;

	case STAGE_2ND: {
		////////////////////////////////////////////////////////////
		// TODO: Cuốn vào đến khi cảm biến 3 phát hiện.
		// EXCEPTION: Giấy ngắn (cảm biến 1 phát hiện giấy cuốn vào, 
		// cảm biến 1 phát hiện hết giấy, cảm biến 2 phát hiện có 
		// giấy, cảm biến 3 ko có giấy).
		////////////////////////////////////////////////////////////
	#if 0
		///////////////////////////////////////////////
		// TODO: Giấy ngắn, báo chụp ảnh, đợi confirm 
		// rồi nhả giấy ra
		///////////////////////////////////////////////
		if (readFrontSensor() == PAPER_UNDETECTED && readMidSensor() == PAPER_DETECTED) {
			LEDFLASH.OnState();
			ENGINES.setMOTORS(MOTOR_FRONT, STOPPING);
			sendPCREQandWait(MPU_REQSCREENSHOT);
			scannerStageRun = STAGE_4TH;
			break;
		}
	#else
		///////////////////////////////////////////////
		// TODO: Giấy ngắn, báo chụp ảnh, đợi confirm 
		// rồi nhả giấy ra
		///////////////////////////////////////////////
		if (isPaperShortDetected == true || 
			(readFrontSensor() == PAPER_UNDETECTED && readMidSensor() == PAPER_DETECTED))
		{
			APP_PRINT("----->/CASE/ Short paper\r\n");
			LEDFLASH.OnState();
			ENGINES.setMOTORS(MOTOR_FRONT, STOPPING);
			sendPCREQandWait(MPU_REQSCREENSHOT);
			scannerStageRun = STAGE_4TH;
			break;
		}
	#endif

		if (readRearSensor() == PAPER_DETECTED) { /* Giấy dài */
			APP_PRINT("----->/CASE/ Long paper\r\n");
			ENGINES.setMOTORS(MOTOR_FRONT, STOPPING);
			LEDFLASH.OnState();
			sendPCREQandWait(MPU_REQSCREENSHOT);
			scannerStageRun = STAGE_3RD;
			EXTI_DetectOutOfPaper(true);
			EXTI_DetectShortPaper(false);
		}
	}
	break;

	case STAGE_3RD: {
		/////////////////////////////////////////////////////////
		// TODO: Cuộn motor để đưa giấy vào trong và gửi yêu cầu
		// chụp hình tới MPU
		////////////////////////////////////////////////////////
		if (readFrontSensor() == PAPER_UNDETECTED && readMidSensor() == PAPER_UNDETECTED) { /* Giấy đã được cuộn hết */
			scannerStageRun = STAGE_4TH;
		}
		else {
			ENGINES.setMOTORS(DUOMOTORS, SCROLL_FORDWARD);

			uint32_t timStamp = millisTick();
			while (!customDelay(usrAdjust.delayVal, timStamp));
			ENGINES.setMOTORS(DUOMOTORS, STOPPING);
			sendPCREQandWait(MPU_REQSCREENSHOT);
		}
	}
	break;

	case STAGE_4TH: {
		/////////////////////////////////////////////////////////
		// TODO: Gửi command hết giấy mà không cần chờ phản hồi
		/////////////////////////////////////////////////////////
		const char *notifyMsg = MPU_OUTOFPAPPER;
		putMPUMessage(notifyMsg);
		APP_DBG(TAG, "Command - %s", notifyMsg);
		scannerStageRun = STAGE_5TH;
	}
	break;

	case STAGE_5TH: {
		///////////////////////////////////////////////////////////
		// TODO: Sau khi gửi message hết giấy thì nhả giấy ra khỏi 
		// bộ cuốn giấy
		///////////////////////////////////////////////////////////
		EXTI_DetectOutOfPaper(false);
		EXTI_DetectEndpointPaper(false);
		EXTI_DetectShortPaper(false);
		taskPostPureMsg(SL_TASK_DEVMANAGER_ID, SL_DMANAGER_CONVEYOR_CLEANUP);
		scannerStageRun = STAGE_IDLE;
	}
	break;
	
	default:
	break;
	}

	///////////////////////////////////////////////////////////////////////////
	// TODO: Nếu động cơ đang ở trạng thái chạy, đọc xung từ encoder để xác 
	// định động cơ có đang chạy hay không.
	// EXCEPTION: Kẹt giấy -> động cơ được cho quay nhưng encoder đọc ko thấy 
	// giá trị thì dừng động cơ báo lỗi fault. Led FAULT sáng lên gửi message 
	// “#3\r\n”.
	///////////////////////////////////////////////////////////////////////////
	MOTORS_Monitoring();
}

void MOTORS_Monitoring() {
#define MOTORS_MONITOR_INTERVAL		(700)
#define PAPER_JAM_ASSERT_COUNTER	(3)
	bool isFault = false;
	bool isMOTORRun;
	static uint8_t assertPaperJameCnt = 0;
	static uint32_t timBlinky = millisTick();

	if (millisTick() - timBlinky > MOTORS_MONITOR_INTERVAL) {
		timBlinky = millisTick();
		////////////////////
		// MOTOR Front
		///////////////////
		if (ENGINES.readMOTORStateCtl(MOTOR_FRONT) != STOPPING) {
			isMOTORRun = ENGINES.isRun(MOTOR_FRONT);
			if (!isMOTORRun) {
				APP_PRINT("MOTOR_FRONT -> STOPPED\r\n");
				isFault = true;
			}
			else {
				APP_PRINT("MOTOR_FRONT -> RUNNING\r\n");
			}
		}
		////////////////////
		// MOTOR Rear
		///////////////////
		if (ENGINES.readMOTORStateCtl(MOTOR_REAR) != STOPPING) {
			isMOTORRun = ENGINES.isRun(MOTOR_REAR);
			if (!isMOTORRun) {
				APP_PRINT("MOTOR_REAR -> STOPPED\r\n");
				isFault = true;
			}
			else {
				APP_PRINT("MOTOR_REAR -> RUNNING\r\n");
			}
		}
		
		if (isFault) {
			++assertPaperJameCnt;
			if (assertPaperJameCnt == PAPER_JAM_ASSERT_COUNTER) { /* Trong 3s, nếu động cơ vẫn kẹt thì report */
				APP_DBG(TAG, "Paper Jam has detected !!!");
				makeBeepSound(100);
				/* Đưng động cơ */
				ENGINES.setMOTORS(DUOMOTORS, STOPPING);
				/* LED Fault sáng */
				LEDFAULT.OnState();
				/* Gửi message “#3\r\n” và chờ response */
				const char *repMsg = MPU_PAPERJAM;
				sendPCREQandWait(repMsg);
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
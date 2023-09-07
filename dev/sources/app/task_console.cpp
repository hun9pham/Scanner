#include <stdbool.h>

#include "ak.h"
#include "heap.h"
#include "task.h"
#include "message.h"
#include "timer.h"

#include "cmd_line.h"
#include "ring_buffer.h"
#include "xprintf.h"

#include "app.h"
#include "app_dbg.h"
#include "app_data.h"
#include "app_bsp.h"
#include "task_list.h"
#include "task_console.h"

#include "platform.h"
#include "io_cfg.h"
#include "sys_cfg.h"
#include "sys_ctl.h"

#include "sys_dbg.h"

#define TAG	"TaskConsole"

/* Extern variables ----------------------------------------------------------*/
extern sysStructInfo_t systemIn4;

/* Private function prototypes -----------------------------------------------*/
static int8_t csInfo(uint8_t* argv);
static int8_t csHelp(uint8_t* argv);
static int8_t csRst(uint8_t* argv);
static int8_t csFatal(uint8_t* argv);
static int8_t csDbg(uint8_t *argv);

static cmdLineStruct_t lgnCmdTable[] = {
	/*------------------------------------------------------------------------------*/
	/*									System command								*/
	/*------------------------------------------------------------------------------*/
	{(const int8_t*)"info",		csInfo,	    (const int8_t*)"System information"		},
	{(const int8_t*)"help",		csHelp,		(const int8_t*)"Help information"		},
	{(const int8_t*)"rst",		csRst,		(const int8_t*)"Reset system"			},
	{(const int8_t*)"fatal"	,	csFatal,	(const int8_t*)"Fatal information"		},
	{(const int8_t*)"dbg"	,	csDbg,		(const int8_t*)"Debugging"				},
	/*------------------------------------------------------------------------------*/
	/*									End of table								*/
	/*------------------------------------------------------------------------------*/
	{(const int8_t*)0,			(pfCmdFunc)0,			(const int8_t*)0			}
};

/* Private variables ----------------------------------------------------------*/

/* Function implementation ---------------------------------------------------*/
void TaskConsole(ak_msg_t *msg) {
	switch (msg->sig) {
	case SL_CONSOLE_HANDLE_CMD_LINE: {
		switch (cmdLineParser(lgnCmdTable, ' ')) {
		case CMD_SUCCESS:
		break;
		case CMD_NOT_FOUND:
		break;
		case CMD_TOO_LONG:
		break;
		case CMD_TBL_NOT_FOUND:
		break;

		default:
		break;
		}
	}
	break;

	default:
	break;
	}
}

void TaskPollConsole(void) {
	extern ringBufferChar_t ConsoleRx;

	uint8_t ch;
	while (!isRingBufferCharEmpty(&ConsoleRx)) {
		ch = ringBufferCharGet(&ConsoleRx);

		if (ch == '\r' || ch == '\n') {
			APP_PRINT("\r\n");
		
			taskPostPureMsg(SL_TASK_CONSOLE_ID, SL_CONSOLE_HANDLE_CMD_LINE);

			APP_PRINT("- ");
		}
		else if (ch == 8 || ch == 127) { /* Backspace & Delelte */
			cmdLineClrChar();
		}
		else {
			cmdLinePutChar(ch);
		}
	}
}

/*----------------------------------------------------------------------------*/
int8_t csInfo(uint8_t* argv) {
	(void)argv;
	
	switch(getCmdLineParserCounter()) {
	case 1: {
		if (strcmp((const char*)cmdLineGetAttr(1), (const char*)"heap") == 0) {
			APP_PRINT("Heap information\n");
			APP_PRINT(" .Size: %d\n", getTotalHeapSize());
			APP_PRINT(" .Used: %d\n", getTotalHeapUsed());
			APP_PRINT(" .Free: %d\n", getTotalHeapFree());
		}
	}
	break;

	default: {
		APP_PRINT("\n[SYSTEM INFORMATION]\n");
		APP_PRINT("1. MCU Platform:\t\t%s\n", 			MCU_PLATFROM);
		APP_PRINT("\tCPU clock:\t\t%d Hz\n", 			systemIn4.CpuClk);
		APP_PRINT("\tTime tick:\t\t%d ms\n", 			systemIn4.Tick);
		APP_PRINT("\tBaudrate:\t\t%d bps\n", 			systemIn4.Baudrate);
		APP_PRINT("\tFlash used:\t\t%d bytes\n", 		systemIn4.flashUsed);
		APP_PRINT("\tSram used:\t\t%d bytes\n", 		systemIn4.sramUsed);
		APP_PRINT("\t\t.data:\t\t%d bytes\n", 			systemIn4.dataUsed);
		APP_PRINT("\t\t.bss:\t\t%d bytes\n", 			systemIn4.bssUsed);
		APP_PRINT("\t\tStack avaiable:\t%d bytes\n", 	systemIn4.stackRemain);
		APP_PRINT("\t\tHeap length:\t%d bytes\n", 		systemIn4.heapSize);
		APP_PRINT("2. Hardware version:\t%s\n", 		systemIn4.Hardware);
		APP_PRINT("3. Firmware version:\t%s\n", 		systemIn4.Firmware);
		APP_PRINT("\n");
	}
	break;
	}
	
	return 0;
}	

int8_t csHelp(uint8_t* argv) {	
	APP_PRINT("\r\nHelp commands:\r\n");
	for (uint8_t id = 0; id < sizeof(lgnCmdTable) / sizeof(lgnCmdTable[0]) - 1; ++id) {
		APP_PRINT("  -%s: %s\r\n", lgnCmdTable[id].cmd, lgnCmdTable[id].Str);
	}

	return 0;
}

int8_t csRst(uint8_t* argv) {
	(void)argv;
	softReset();

	return 0;
}

int8_t csFatal(uint8_t* argv) {
	switch (*(argv + 6)) {
	case 't': {
		FATAL("TEST", 0x01);
	}
	break;

	case 'r': {
		if (fatalClear()) {
			APP_PRINT("Fatal clear\n");
		}
	}
	break;

	case 'l': {
		fatalLog_t *fatalLogCs = fatalRead();

		APP_PRINT("\n\n");
		APP_PRINT("[TIMES] FATAL:   %d\n", fatalLogCs->fatalTimes);
		APP_PRINT("[TIMES] RESTART: %d\n", fatalLogCs->restartTimes);

		APP_PRINT("\n");
		APP_PRINT("[FATAL] TYPE: %s\n",	fatalLogCs->Str);
		APP_PRINT("[FATAL] CODE: 0x%02X\n",	fatalLogCs->Code);

		APP_PRINT("\n");
		APP_PRINT("TASK\n");
		APP_PRINT("[SCHEDULER] ID: %d, PRI: %d\n", fatalLogCs->CurrentTaskId, fatalLogCs->CurrentTaskPrio);

		APP_PRINT("\n");
		APP_PRINT("MESSAGE\n");
		APP_PRINT("[OB] TASK: %d\n", fatalLogCs->DesTaskId);
		APP_PRINT("[OB] SIG: %d\n", fatalLogCs->Sig);
		APP_PRINT("[OB] TYPE: 0x%x\n", fatalLogCs->Type);
		APP_PRINT("[OB] REF_COUNT: %d\n", fatalLogCs->RefCount);

		APP_PRINT("\n");
		APP_PRINT("[RESET] BY: %s\n",	fatalLogCs->reasonRst);
		APP_PRINT("\n");
	}
	break;

	default: {
		APP_PRINT("\n<Fatal commands>\n");
		APP_PRINT("Usage:\n");
		APP_PRINT("  fatal [options]\n");
		APP_PRINT("Options:\n");
		APP_PRINT("  l: Fatal log\n");
		APP_PRINT("  r: Fatal clear\n\n");
	}
	break;
	}

	return 0;
}

int8_t csDbg(uint8_t *argv) {
	switch (*(argv + 4)) {
	case 'a': {
		APP_DBG(TAG, "[START] Blink all LEDs");
		for (uint8_t id = 0; id < 8; ++id) {
			LEDFLASH.Blinking();
			LEDSTATUS.Blinking();
			LEDDIR.Blinking();
			LEDFAULT.Blinking();
			delayMillis(100);
		}
		APP_DBG(TAG, "[STOP] Blink all LEDs");
	}
	break;

	case 'b': {
		APP_DBG(TAG, "[START] Motor front test in 2s");
		ENGINES.setMOTORS(MOTOR_FRONT, SCROLL_FORDWARD);
		delayMillis(2000);
		ENGINES.setMOTORS(MOTOR_FRONT, STOPPING);
		APP_DBG(TAG, "[STOP] Motor front test in 2s");
	}
	break;

	case 'c': {
		APP_DBG(TAG, "[START] Motor rear test in 2s");
		ENGINES.setMOTORS(MOTOR_REAR, SCROLL_FORDWARD);
		delayMillis(2000);
		ENGINES.setMOTORS(MOTOR_REAR, STOPPING);
		APP_DBG(TAG, "[STOP] Motor rear test in 2s");
	}
	break;

	case 'd': {
		APP_DBG(TAG, "[START] Read inputs in 5s");
		uint32_t now = millisTick();
		while (millisTick() - now < 5000) {
			if (readInput1() == 0) {
				APP_PRINT("->Input[1] active 0\r\n");
			}
			if (readInput2() == 0) {
				APP_PRINT("->Input[2] active 0\r\n");
			}
			if (readInput3() == 0) {
				APP_PRINT("->Input[3] active 0\r\n");
			}
			watchdogRst();
		}
		APP_DBG(TAG, "[STOP] Read inputs in 5s");
	}
	break;

	case 'e': {
		APP_DBG(TAG, "[DBG] Send \'Hello world\' to MPU");
		const char *testStr = (const char*)"Hello world\r\n";
		putMPUMessage(testStr);
	}
	break;

	case 'f': {
		extern UserSetting_t usrAdjust;
		char *pt = (char*)(argv + 6);
		uint32_t delayCalib = atoi(pt);
		if (delayCalib >= 500 && delayCalib <= 3000) {
			APP_PRINT("Set delay scrolling - %d", delayCalib);
			usrAdjust.delayVal = delayCalib;
		}
		else {
			delayCalib = 500;
			APP_PRINT("Invalid delay value - %d", delayCalib);
		}
		usrAdjust.delayVal = delayCalib;
		EEPROM_Write(eepromUSER_SETTING_ADDR, (uint8_t*)&usrAdjust, sizeof(UserSetting_t));
	}
	break;

	default: {
		APP_PRINT("\n<Debugging commands>\n");
		APP_PRINT("Usage:\n");
		APP_PRINT("  dbg [options]\n");
		APP_PRINT("Options:\n");
		APP_PRINT("  a: Blink all LEDs\n");
		APP_PRINT("  b: Motor front test in 2s\n");
		APP_PRINT("  c: Motor rear test in 2s\n");
		APP_PRINT("  d: Read inputs in 5s\n");
		APP_PRINT("  f: Calibrate delay scrolling [500:3000]. Example: dbg f 500\n");
	}
	break;
	}
	return 0;
}
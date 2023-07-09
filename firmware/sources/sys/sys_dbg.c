#include <stdio.h>
#include <string.h>

#include "ak_dbg.h"
#include "task.h"
#include "message.h"

#include "xprintf.h"

#include "app.h"

#include "io_cfg.h"
#include "sys_cfg.h"
#include "sys_ctl.h"

#include "sys_log.h"
#include "sys_dbg.h"

#define TAG	"SysDbg"

/* Extern variables ----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static fatalLog_t fatalLog;

/* Private function prototypes -----------------------------------------------*/

/* Function implementation ---------------------------------------------------*/
void fatalInit(void) {
	char *rstReason = getRstReason(true);
	
	SYS_LOG(TAG, "-RstReason: %s\n", rstReason);

#if LOG_CONTAINER_OPT
	flashRead(FLASH_SECTOR_DBG_FATAL_LOG, (uint8_t *)&fatalLog, sizeof(fatalLog_t));
#else
	EEPROM_Read(EEPROM_FATAL_LOG_BIASADDR, (uint8_t *)&fatalLog, sizeof(fatalLog_t));
#endif

	if (fatalLog.magicNum != FATAL_LOG_MAGIC_NUMBER) {
		memset(&fatalLog, 0, sizeof(fatalLog_t));
		fatalLog.magicNum = FATAL_LOG_MAGIC_NUMBER;
	}
	else {
		++(fatalLog.restartTimes);
	}
	strcpy(fatalLog.reasonRst, rstReason);
#if LOG_CONTAINER_OPT
	flashEraseSector(FLASH_SECTOR_DBG_FATAL_LOG);
	flashWrite(FLASH_SECTOR_DBG_FATAL_LOG, (uint8_t*)&fatalLog, sizeof(fatalLog_t));
#else
	EEPROM_Clear(EEPROM_FATAL_LOG_BIASADDR, sizeof(fatalLog_t));
	EEPROM_Write(EEPROM_FATAL_LOG_BIASADDR, (uint8_t*)&fatalLog, sizeof(fatalLog_t));
#endif
	
}

bool fatalClear() {
	uint8_t ret = false;

	memset((uint8_t*)&fatalLog, 0, sizeof(fatalLog_t));
	fatalLog.magicNum = FATAL_LOG_MAGIC_NUMBER;
#if LOG_CONTAINER_OPT
	flashEraseSector(FLASH_SECTOR_DBG_FATAL_LOG);
	if (flashWrite(FLASH_SECTOR_DBG_FATAL_LOG, (uint8_t*)&fatalLog, sizeof(fatalLog_t))) {
		ret = true;
	}
#else
	EEPROM_Clear(EEPROM_FATAL_LOG_BIASADDR, sizeof(fatalLog_t));
	EEPROM_Write(EEPROM_FATAL_LOG_BIASADDR, (uint8_t*)&fatalLog, sizeof(fatalLog_t));
#endif
	return ret;
}

void fatalApp(const int8_t* s, uint8_t c) {
	DISABLE_INTERRUPTS();
	
	watchdogRst();
	SYS_LOG(TAG, "%s\t%x", s, c);

#if LOG_CONTAINER_OPT
	flashRead(FLASH_SECTOR_DBG_FATAL_LOG, (uint8_t*)&fatalLog, sizeof(fatalLog_t));
#else
	EEPROM_Read(EEPROM_FATAL_LOG_BIASADDR, (uint8_t*)&fatalLog, sizeof(fatalLog_t));
#endif

	++(fatalLog.fatalTimes);
	memset(fatalLog.Str, 0, 10);

	strcpy((char*)fatalLog.Str, (const char*)s);
	fatalLog.Code = c;

	task_t *fatalTask = get_current_task_info();
	fatalLog.CurrentTaskId = fatalTask->id;
	fatalLog.CurrentTaskPrio = fatalTask->pri;

	ak_msg_t *fatalMsg	= get_current_active_object();
	fatalLog.DesTaskId = fatalMsg->des_task_id;
	fatalLog.Sig 		= fatalMsg->sig;
	fatalLog.RefCount	= get_msg_ref_count(fatalMsg);
	fatalLog.Type 		= get_msg_type(fatalMsg);

#if LOG_CONTAINER_OPT
	flashEraseSector(FLASH_SECTOR_DBG_FATAL_LOG);
	flashWrite(FLASH_SECTOR_DBG_FATAL_LOG, (uint8_t*)&fatalLog, sizeof(fatalLog_t));
#else
	EEPROM_Clear(EEPROM_FATAL_LOG_BIASADDR, sizeof(fatalLog_t));
	EEPROM_Write(EEPROM_FATAL_LOG_BIASADDR, (uint8_t*)&fatalLog, sizeof(fatalLog_t));
#endif

#if defined(RELEASE)
	softReset();
#else

	while(1) {
		char Ch = getConsoleChar();

		switch (Ch) {
		case 'r': {
			softReset();
		}
		break;

		case 'l': {
			SYS_PRINT("\n\n");
			SYS_LOG(TAG, "[TIMES] FATAL:   %d", fatalLog.fatalTimes);
			SYS_LOG(TAG, "[TIMES] RESTART: %d", fatalLog.restartTimes);

			SYS_PRINT("\n");
			SYS_LOG(TAG, "[FATAL] TYPE: %s",	fatalLog.Str);
			SYS_LOG(TAG, "[FATAL] CODE: 0x%02X",	fatalLog.Code);

			SYS_PRINT("\n");
			SYS_LOG(TAG, "TASK");
			SYS_LOG(TAG, "[SCHEDULER] ID: %d, PRI: %d", fatalLog.CurrentTaskId, fatalLog.CurrentTaskPrio);

			SYS_PRINT("\n");
			SYS_LOG(TAG, "MESSAGE");
			SYS_LOG(TAG, "[OB] TASK: %d", fatalLog.DesTaskId);
			SYS_LOG(TAG, "[OB] SIG: %d", fatalLog.Sig);
			SYS_LOG(TAG, "[OB] TYPE: 0x%x", fatalLog.Type);
			SYS_LOG(TAG, "[OB] REF_COUNT: %d", fatalLog.RefCount);
		}
		break;

		default:
		break;
		}
		
		watchdogRst();
		ledLifeToggle();
		delayMillis(50);
	}

#endif
}

fatalLog_t *fatalRead() {
#if LOG_CONTAINER_OPT
	flashRead(FLASH_SECTOR_DBG_FATAL_LOG, (uint8_t *)&fatalLog, sizeof(fatalLog_t));
#else
	EEPROM_Read(EEPROM_FATAL_LOG_BIASADDR, (uint8_t*)&fatalLog, sizeof(fatalLog_t));
#endif
	return &fatalLog;
}

void fatalGet(fatalLog_t *params) {
#if LOG_CONTAINER_OPT
	flashRead(FLASH_SECTOR_DBG_FATAL_LOG, (uint8_t *)&fatalLog, sizeof(fatalLog_t));
#else
	EEPROM_Read(EEPROM_FATAL_LOG_BIASADDR, (uint8_t*)&fatalLog, sizeof(fatalLog_t));
#endif
	memcpy(params, &fatalLog, sizeof(fatalLog_t));
}

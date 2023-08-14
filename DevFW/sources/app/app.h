#ifndef __APP_H
#define __APP_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#include "ak.h"

/*----------------------------------------------------------------------------*
 *  DECLARE: SL_TASK_CONSOLE_ID
 *  Note: Message signals
 *----------------------------------------------------------------------------*/
/* Define timer */

/* Define signal */
enum {
    SL_CONSOLE_HANDLE_CMD_LINE = AK_USER_DEFINE_SIG,
};

/*----------------------------------------------------------------------------*
 *  DECLARE: SL_TASK_IF_ID
 *  Note: Message signals
 *----------------------------------------------------------------------------*/
#define IF_TYPE_CPU_SERIAL_MT					( 120 )
#define IF_TYPE_CPU_SERIAL_SL					( 121 )

/* Define timer */

/* Define signal */
enum {
    SL_IF_PURE_MSG_IN = AK_USER_DEFINE_SIG,
    SL_IF_PURE_MSG_OUT,
    SL_IF_COMMON_MSG_IN	,
    SL_IF_COMMON_MSG_OUT,
    SL_IF_DYNAMIC_MSG_IN,
    SL_IF_DYNAMIC_MSG_OUT
};

/*----------------------------------------------------------------------------*
 *  DECLARE: SL_TASK_CPU_SERIAL_IF_ID
 *  Note: Message signals
 *----------------------------------------------------------------------------*/
/* Define timer */

/* Define signal */
enum {
    SL_CPU_SERIAL_IF_INIT = AK_USER_DEFINE_SIG,
    SL_CPU_SERIAL_IF_PURE_MSG_OUT,
    SL_CPU_SERIAL_IF_COMMON_MSG_OUT,
    SL_CPU_SERIAL_IF_DYNAMIC_MSG_OUT,
    SL_CPU_SERIAL_IF_PURE_MSG_IN,
    SL_CPU_SERIAL_IF_COMMON_MSG_IN,
    SL_CPU_SERIAL_IF_DYNAMIC_MSG_IN,
};

/*----------------------------------------------------------------------------*
 *  DECLARE: SL_TASK_SYSTEM_ID
 *  Note: Message signals
 *----------------------------------------------------------------------------*/
/* Define timer */
#define SL_SYSTEM_ALIVE_NOTIFY_INTERVAL     ( 1000 )
#define SL_SYSTEM_STARTUP_INTERVAL          ( 150 )

/* Define signal */
enum {
    SL_SYSTEM_ALIVE_NOTIFY = AK_USER_DEFINE_SIG,
    SL_SYSTEM_REBOOT_REQ,
};

/*----------------------------------------------------------------------------*
 *  DECLARE: SL_TASK_DEVMANAGER_ID
 *  Note: Message signals
 *----------------------------------------------------------------------------*/
/* Define timer */

/* Define signal */
enum {
    SL_DMANAGER_CONVEYOR_CLEANUP = AK_USER_DEFINE_SIG,
    SL_DMANAGER_ENTRY_IDLING,
    SL_DMANAGER_START_WORKFLOW_REQ,
    SL_DMANAGER_PROCEDURE_CALL_REQ,
    SL_DMANAGER_PROCEDURE_CALL_RESP,
    SL_DMANAGER_HANDLE_EXCP_PAPER_JAM,
};

/*----------------------------------------------------------------------------*
 *  DECLARE: Common definitions
 *  Note: 
 *----------------------------------------------------------------------------*/
#ifndef APP_START_ADDR
#define APP_START_ADDR                  ( 0x08003000 )
#define BOOT_START_ADDR                 ( 0x08000000 )
#endif

#define NORMAL_START_ADDRESS			(APP_START_ADDR)
#define BOOT_START_ADDRESS			    (BOOT_START_ADDR)

#ifdef RELEASE
#define MCU_PLATFROM                     "Stm32l151c8t6"
#define FIRMWARE_VERSION 		         "v1.1"
#define HARDWARE_VERSION 		         "v1.0"
#else
#define MCU_PLATFROM                     "STM32L151C8T6"
#define FIRMWARE_VERSION 		         "v1.0"
#define HARDWARE_VERSION 		         "v2.0"
#endif

/* Define Tinker board message */
#define MPU_NOTIFYLOGIN      (const char*)"2\n"
#define MPU_REQSCREENSHOT    (const char*)"#1\r\n"
#define MPU_CORMFIRM         (const char*)"1\n"
#define MPU_OUTOFPAPPER      (const char*)"#2\r\n"
#define MPU_PAPERJAM         (const char*)"#3\r\n"
#define MPU_WEBTIMEOUT       (const char*)"0\n"

/* Function prototypes -------------------------------------------------------*/
extern int main_app();

#ifdef __cplusplus
}
#endif

#endif /* __APP_H__ */

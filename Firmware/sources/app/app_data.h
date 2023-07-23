#ifndef __APP_DATA_H
#define __APP_DATA_H

#include <stdint.h>

/*----------------------------------------------------------------------------*
 *  DECLARE: Common definitions
 *  Note: Data using for all task will be declare in here
 *----------------------------------------------------------------------------*/
#define MAX_MPU_MSG_RECV_LEN        (8)

#define eepromUSER_SETTING_ADDR     (50)
#define delaySCROLLING_MAGICNUM     (0x11223344)

/* Enumarics -----------------------------------------------------------------*/
typedef enum eDevCtlStage {
    STAGE_IDLE = 0,
    STAGE_1ST,
    STAGE_2ND,
    STAGE_3RD,
    STAGE_4TH,
    STAGE_5TH,
    EXCEPTION_CASE,
} DevMngCtlStage_t;

/* Typedef -------------------------------------------------------------------*/
typedef struct t_MPU_IncomeMsg{
    uint8_t buf[MAX_MPU_MSG_RECV_LEN];
    uint8_t ind;
} MPU_IncomMsg_t;

typedef struct t_UserSetting {
    uint32_t magicNum;
    uint32_t delayVal;
} UserSetting;

/* Extern variables ----------------------------------------------------------*/
extern DevMngCtlStage_t devStagePolling;
extern MPU_IncomMsg_t MPU_IncomMsg;
extern UserSetting usrAdjust;

/* Function prototypes -------------------------------------------------------*/

#endif /* __APP_DATA_H */

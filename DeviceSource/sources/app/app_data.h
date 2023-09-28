#ifndef __APP_DATA_H
#define __APP_DATA_H

#include <stdint.h>

/*----------------------------------------------------------------------------*
 *  DECLARE: Common definitions
 *  Note: Data using for all task will be declare in here
 *----------------------------------------------------------------------------*/
#define MAX_MPU_MSG_RECV_LEN        (8)

#define eepromUSER_SETTING_ADDR     (50)
#define delaySCROLLING_MAGICNUM     (0x11223345)

/* Enumarics -----------------------------------------------------------------*/

/* Typedef -------------------------------------------------------------------*/
typedef struct t_MPU_IncomeMsg{
    uint8_t buf[MAX_MPU_MSG_RECV_LEN];
    uint8_t ind;
} MPU_IncomMsg_t;

typedef struct t_UserSetting {
    uint32_t magicNum;
    uint32_t delayVal;
} UserSetting_t;

/* Extern variables ----------------------------------------------------------*/
extern MPU_IncomMsg_t MPU_IncomMsg;
extern UserSetting_t usrAdjust;

/* Function prototypes -------------------------------------------------------*/

#endif /* __APP_DATA_H */

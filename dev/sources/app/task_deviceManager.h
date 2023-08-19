#ifndef __TASK_DEVICE_MANAGER_H
#define __TASK_DEVICE_MANAGER_H

/*----------------------------------------------------------------------------*
 *  DECLARE: Common definitions
 *  Note: 
 *----------------------------------------------------------------------------*/
#define PAPER_DETECTED      (0)
#define PAPER_UNDETECTED    (1)

/* Typedef -------------------------------------------------------------------*/
typedef enum eDevCtlStage {
    STAGE_IDLE = 0,
    STAGE_1ST,
    STAGE_2ND,
    STAGE_3RD,
    STAGE_4TH,
    STAGE_5TH,
    STAGE_EXCP = 0xFF,
} DevMngCtlStage_t;

/* Extern variables ----------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

#endif /* __TASK_DEVICE_MANAGER_H */

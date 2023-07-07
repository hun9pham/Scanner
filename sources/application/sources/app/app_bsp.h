#ifndef __APP_BSP_H
#define __APP_BSP_H

#include <stdint.h>

#include "led.h"

/*----------------------------------------------------------------------------*
 *  DECLARE: Common definitions
 *  Note: Data using for all task will be declare in here
 *----------------------------------------------------------------------------*/

/* Enumarics -----------------------------------------------------------------*/

/* Typedef -------------------------------------------------------------------*/

/* Extern variables ----------------------------------------------------------*/
extern led_t LedLife;
extern led_t LedFlash;
extern led_t LedStatus;
extern led_t LedDir;
extern led_t LedFault;

/* Function prototypes -------------------------------------------------------*/
extern void appBspInit();

#endif /* __APP_BSP_H */

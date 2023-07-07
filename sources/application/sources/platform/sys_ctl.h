#ifndef __SYS_CTL_H
#define __SYS_CTL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

#include "sys_boot.h"

/*----------------------------------------------------------------------------*
 *  DECLARE: System control defines
 *  Note: 
 *----------------------------------------------------------------------------*/
#define SYS_POWER_ON_RESET			        ( 0x00 )
#define SYS_NON_POWER_ON_RESET		        ( 0x01 )

/* Enumarics -----------------------------------------------------------------*/

/* Typedef -------------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/
extern void softReset();
extern void watchdogRst(void);

extern void TIM6_Disable(void);
extern void TIM6_Enable(void);

extern void delayAsm(uint32_t __cnt);
extern void delayTickMs(uint16_t ms);
extern void delayTickUs(uint32_t us);
extern uint32_t millisTick(void);
extern uint32_t microsTick(void);
extern void delayMicros(uint32_t t);
extern void delayMillis(uint32_t t);

extern void internalFlashUnlock(void);
extern void internalFlashLock(void);
extern void internalFlashEraseCalc(uint32_t addr, uint32_t len);
extern uint8_t internalFlashProgramCalc(uint32_t addr, uint8_t* buf, uint32_t len);
extern void internalFlashClearFlag(void);

extern uint8_t EEPROM_Write(uint32_t biasAddr, uint8_t *buf, uint16_t len);
extern void EEPROM_Read(uint32_t biasAddr, uint8_t *buf, uint16_t len);
extern void EEPROM_Clear(uint32_t biasAddr, uint16_t len);

extern uint8_t getConsoleChar(void);
extern void putConsoleChar(uint8_t ch);

extern void getFirmwareHeader(firmwareHeader_t*);
extern char* getRstReason(bool clr);

#ifdef __cplusplus
}
#endif

#endif /* __SYS_CTL_H */

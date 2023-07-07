#ifndef __SYS_DBG_H
#define __SYS_DBG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "platform.h"

#define LOG_CONTAINER_OPT           (0) /* Opt: 0-EEPROM, 1-FlashExternal */

#define FATAL_LOG_MAGIC_NUMBER      ( 0xFA )
#define FATAL(s, c)                 fatalApp((const int8_t*)s, (uint8_t)c)
#define FLASH_SECTOR_DBG_FATAL_LOG  (FLASH_BLOCK_START_ADDR)
#define EEPROM_FATAL_LOG_BIASADDR   (0)

/* Typedef -------------------------------------------------------------------*/
typedef struct {
    char Str[10];
    char reasonRst[8];
    uint8_t	Code;
    uint8_t CurrentTaskId;
    uint8_t CurrentTaskPrio;
    uint8_t DesTaskId;
    uint8_t Sig;
    uint8_t RefCount;
    uint8_t Type;
    uint8_t magicNum;
    uint32_t fatalTimes;
    uint32_t restartTimes;
} fatalLog_t;

/* Extern variables ----------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/
extern void fatalInit(void);
extern bool fatalClear(void);
extern void fatalApp(const int8_t* s, uint8_t c);
extern fatalLog_t *fatalRead(void);
extern void fatalGet(fatalLog_t *params);

#ifdef __cplusplus
}
#endif

#endif  /* __SYS_DBG_H */

#ifndef __SYS_CFG_H
#define __SYS_CFG_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#include "stm32xx_header.h"

/*----------------------------------------------------------------------------*
 *  DECLARE: USART Communication boards interface defines
 *  Note: 
 *----------------------------------------------------------------------------*/
#define UART_CONSOLE_CLOCK               	( RCC_APB2Periph_USART1 )
#define UART_CONSOLE                     	( USART1 )
#define UART_CONSOLE_IRQn                	( USART1_IRQn )
#define UART_CONSOLE_BAUD            		( 460800 )

#define IO_UART_CONSOLE_CLOCK				( RCC_AHBPeriph_GPIOA )
#define IO_UART_CONSOLE_PORT             	( GPIOA )
#define IO_UART_CONSOLE_TX_PIN           	( GPIO_Pin_9 )
#define IO_UART_CONSOLE_RX_PIN           	( GPIO_Pin_10 )

/* Typedef -------------------------------------------------------------------*/
typedef struct {
	uint32_t CpuClk;
	uint32_t Tick;
	uint32_t Baudrate;
	uint32_t flashUsed;
	uint32_t sramUsed;
	uint32_t dataUsed;
	uint32_t bssUsed;
	uint32_t stackRemain;
	uint32_t heapSize;
} sysStructInfo_t;

/* Extern variables ----------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/
extern void initSystemClock(void);
extern void initSystemTick(void);
extern void ConsoleIfInit(void);
extern void watchdogInit(void);


#ifdef __cplusplus
}
#endif

#endif /* __SYS_CFG_H */

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "app.h"

#include "xprintf.h"
#include "ring_buffer.h"

#include "stm32xx_header.h"
#include "sys_cfg.h"
#include "sys_ctl.h"

#include "sys_log.h"
#include "sys_dbg.h"

/* Extern variables ----------------------------------------------------------*/
sysStructInfo_t systemIn4;
ringBufferChar_t ConsoleRx;

/* Private variables ---------------------------------------------------------*/
static uint8_t ConsoleRxBuf[CONSOLE_BUFFER_SIZE];

/*---------------------------------------------------------------------------*
 *  DECLARE: System configure function
 *  Note:
 *---------------------------------------------------------------------------*/
void initSystemClock() {
	/* Enable the HSI oscillator --------*/
#if 0
	RCC_HSICmd(ENABLE);
	while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET);
#endif

	RCC_PCLK2Config(RCC_HCLK_Div2);
	SystemCoreClockUpdate();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
}

void initSystemTick() {
	SysTick_Config(SystemCoreClock / 1000);
}

void ConsoleIfInit() {
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(UART_CONSOLE_CLOCK, ENABLE);
	RCC_AHBPeriphClockCmd(IO_UART_CONSOLE_CLOCK, ENABLE);

	GPIO_PinAFConfig(IO_UART_CONSOLE_PORT, GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_PinAFConfig(IO_UART_CONSOLE_PORT, GPIO_PinSource10, GPIO_AF_USART1);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin = IO_UART_CONSOLE_TX_PIN;
	GPIO_Init(IO_UART_CONSOLE_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = IO_UART_CONSOLE_RX_PIN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(IO_UART_CONSOLE_PORT, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = UART_CONSOLE_BAUD;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART_CONSOLE, &USART_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = UART_CONSOLE_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_ClearITPendingBit(UART_CONSOLE,USART_IT_RXNE);
	USART_ITConfig(UART_CONSOLE, USART_IT_RXNE, ENABLE);

	USART_Cmd(UART_CONSOLE, ENABLE);

	/* Setup buffer received data from terminal */
	ringBufferCharInit(&ConsoleRx, 
							ConsoleRxBuf, 
							CONSOLE_BUFFER_SIZE
							);

	ENTRY_CRITICAL();
	xfunc_output = (void(*)(int))putConsoleChar;
	EXIT_CRITICAL();
}

/*---------------------------------------------------------------------------*
 *  DECLARE: TIMER 6
 *  Note:
 *---------------------------------------------------------------------------*/
void TIM6_Initial() {
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

	TIM_TimeBaseStructure.TIM_Prescaler = TIM6_PRESCALER_VALUE - 1;
	TIM_TimeBaseStructure.TIM_Period = TIM6_RELOAD_VALUE - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure );

	NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 8;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
	TIM_ClearFlag(TIM6, TIM_IT_Update);

	TIM_SetCounter(TIM6, 0);
	TIM_ITConfig(TIM6, TIM_IT_Update, DISABLE);
	TIM_Cmd(TIM6, DISABLE);
}

void TIM6_Disable() {
	TIM_SetCounter(TIM6, 0);
	TIM_ITConfig(TIM6, TIM_IT_Update, DISABLE);
	TIM_Cmd(TIM6, DISABLE);
}

void TIM6_Enable() {
	TIM_SetCounter(TIM6, 0);
	TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM6, ENABLE);
}

void updateInfoSystem() {
	extern uint32_t _start_flash;
	extern uint32_t _end_flash;
	extern uint32_t _start_ram;
	extern uint32_t _end_ram;
	extern uint32_t _data;
	extern uint32_t _edata;
	extern uint32_t _bss;
	extern uint32_t _ebss;
	extern uint32_t __heap_start__;
	extern uint32_t __heap_end__;
	extern uint32_t _estack;
	extern uint32_t _end_ram;

	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq(&RCC_Clocks);

	systemIn4.CpuClk = RCC_Clocks.HCLK_Frequency;
	systemIn4.Tick = 1;
	systemIn4.Baudrate = UART_CONSOLE_BAUD;
	systemIn4.flashUsed = ((uint32_t)&_end_flash - (uint32_t)&_start_flash) + ((uint32_t)&_edata - (uint32_t)&_data);
	systemIn4.sramUsed = (uint32_t)&_estack - (uint32_t)&_start_ram;
	systemIn4.dataUsed = (uint32_t)&_edata - (uint32_t)&_data;
	systemIn4.bssUsed = (uint32_t)&_ebss - (uint32_t)&_bss;
	systemIn4.stackRemain = (uint32_t)&_estack - (uint32_t)&_end_ram;
	systemIn4.heapSize = (uint32_t)&__heap_end__ - (uint32_t)&__heap_start__;

	strcpy(systemIn4.Hardware, HARDWARE_VERSION);
	strcpy(systemIn4.Firmware, FIRMWARE_VERSION);
}


/*---------------------------------------------------------------------------*
 *  DECLARE: System utilities function
 *  Note:
 *---------------------------------------------------------------------------*/
void putConsoleChar(uint8_t ch) {
	while (USART_GetFlagStatus(UART_CONSOLE, USART_FLAG_TXE) == RESET);
	USART_SendData(UART_CONSOLE, ch);
	while (USART_GetFlagStatus(UART_CONSOLE, USART_FLAG_TC) == RESET);
}

uint8_t getConsoleChar() {
	uint8_t let = '\0';

	if (USART_GetFlagStatus(UART_CONSOLE, USART_FLAG_RXNE) != RESET) {
		let = (uint8_t)USART_ReceiveData(UART_CONSOLE);
	}

	return let;
}

void softReset() {
	NVIC_SystemReset();
}


/*---------------------------------------------------------------------------*
 *  DECLARE: System control delay function
 *  Note:
 *---------------------------------------------------------------------------*/
//----------------------------------------------------------
/*!< ARM Compiler */
//----------------------------------------------------------
//			#if defined   (__CC_ARM) 
//			__asm void
//			delayAsm(uint32_t __cnt)
//			{
//			    subs    r0, #1;
//			    bne     delayAsm;
//			    bx      lr;
//			}
//----------------------------------------------------------


//----------------------------------------------------------
/*!< IAR Compiler */
//----------------------------------------------------------
//			#if defined ( __ICCARM__ ) 
//			void
//			delayAsm(uint32_t __cnt)
//			{
//			    __asm("    subs    r0, #1\n"
//			       "    bne.n   delayAsm\n"
//			       "    bx      lr");
//			}
//----------------------------------------------------------

//----------------------------------------------------------
/*!< GNU Compiler */
//	Delay n microsecond	__cnt = n*(SystemCoreClock / 3000000)
//	Delay n millisecond	__cnt = n*(SystemCoreClock / 3000)
//----------------------------------------------------------
void __attribute__((naked))
delayAsm(uint32_t __cnt)
{
	(void)__cnt;
    __asm("    subs    r0, #1\n"
		"    bne     delayAsm\n"
		"    bx      lr"
		);
}

void delayTickUs(uint32_t us) {
	uint32_t now = microsTick();

	do {
		
	} while(microsTick() - now < us);
}

void delayTickMs(uint16_t ms) {
	uint32_t now = millisTick();

	do {
		
	} while(millisTick() - now < ms);
}

void delayMicros(uint32_t t) {
	uint32_t count = (t * (SystemCoreClock / 1000000)) / 4;

	for (uint32_t i = 0 ; i < count; ++i) {
		count--;
	}
}

void delayMillis(uint32_t t) {
	uint32_t count = ((t * 1000) * (SystemCoreClock / 1000000)) / 4;

	for (uint32_t i = 0 ; i < count; ++i) {
		count--;
	}
}

/*---------------------------------------------------------------------------*
 *  DECLARE: System independent watchdog function
 *  Note:
 *---------------------------------------------------------------------------*/
void watchdogInit() {
	/* Enable the LSI OSC */
	RCC_LSICmd(ENABLE);

	/* Enable PCLK1 for watchdog timer */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);

	/*-------------------------------------------------------------------------*/
	/* IWDG counter clock: 37KHz(LSI) / 256 = 0.144 KHz 					   */
	/* Set counter reload, T = (1/IWDG counter clock) * Reload_counter  = 30s  */
	/*-------------------------------------------------------------------------*/
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(IWDG_Prescaler_256);		
	IWDG_SetReload(0xFFF);						
	IWDG_ReloadCounter();
	IWDG_Enable();
}

void watchdogRst() {
	ENTRY_CRITICAL();
	IWDG_ReloadCounter();
	EXIT_CRITICAL();
}

/*---------------------------------------------------------------------------*
 *  DECLARE: System utilities
 *  Note:
 *---------------------------------------------------------------------------*/
void getFirmwareHeader(firmwareHeader_t* fwHeader) {
	extern uint32_t _start_flash;
	extern uint32_t _end_flash;
	extern uint32_t _data;
	extern uint32_t _edata;

	uint32_t CsCalc = 0;
	uint32_t flashLen = (uint32_t)&_end_flash - (uint32_t)&_start_flash + ((uint32_t)&_edata - (uint32_t)&_data) + sizeof(uint32_t);

	for (uint32_t id = (uint32_t)&(_start_flash); id < ((uint32_t)&(_start_flash) + flashLen); id += sizeof(uint32_t)) {
		CsCalc += *((uint32_t*)id);
	}

	fwHeader->Psk = FIRMWARE_PSK;
	fwHeader->Checksum = (uint16_t)(CsCalc & 0xFFFF);
	fwHeader->binLen = flashLen;
}


char* getRstReason(bool clr) {
	char *rst = NULL;

	if (RCC_GetFlagStatus(RCC_FLAG_PORRST) == SET) {
		rst = (char *)"POR/PWR";
	}
	else if (RCC_GetFlagStatus(RCC_FLAG_SFTRST) == SET) {
		rst = (char *)"SFT";
	}
	else if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) == SET) {
		rst = (char *)"IWDG";
	}
	else if (RCC_GetFlagStatus(RCC_FLAG_WWDGRST) == SET) {
		rst = (char *)"WWDG";
	}
	else if (RCC_GetFlagStatus(RCC_FLAG_LPWRRST) == SET) {
		rst = (char *)"LPWR";
	}
	else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) == SET) {
		rst = (char *)"PINRST";
	}
	else {
		rst = (char *)"UNK";
	}

	if (clr) {
		RCC_ClearFlag();
	}

	return rst;
}

/*---------------------------------------------------------------------------*
 *  DECLARE: System internal flash function
 *  Note:
 *---------------------------------------------------------------------------*/
#ifndef FLASH_PAGE_SIZE
#define FLASH_PAGE_SIZE			( 0x100 ) /* 256Bytes each page */
#endif

void internalFlashUnlock() {
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP |
					FLASH_FLAG_WRPERR | 
					FLASH_FLAG_PGAERR |
					FLASH_FLAG_SIZERR | 
					FLASH_FLAG_OPTVERR | 
					FLASH_FLAG_OPTVERRUSR
					);
}

void internalFlashLock() {
	FLASH_Lock();
}

void internalFlashEraseCalc(uint32_t addr, uint32_t len) {
	uint32_t PageNbr;
	uint32_t id;

	PageNbr = len / FLASH_PAGE_SIZE;

	if ((PageNbr * FLASH_PAGE_SIZE) < len) {
		PageNbr++;
	}

	for (id = 0; id < PageNbr; id++) {
		FLASH_ErasePage(addr + (id * FLASH_PAGE_SIZE));
	}
}

uint8_t internalFlashProgramCalc(uint32_t addr, uint8_t* buf, uint32_t len) {
	uint32_t tmpData;
	uint32_t id = 0;
	FLASH_Status ft = FLASH_BUSY;

	while (id < len) {
		tmpData = 0;

		memcpy(&tmpData, &buf[id], (len - id) >= sizeof(uint32_t) ? sizeof(uint32_t) : (len - id));

		ft = FLASH_FastProgramWord(addr + id, tmpData);

		if(ft == FLASH_COMPLETE) {
			id += sizeof(uint32_t);
		}
		else {
			FLASH_ClearFlag(FLASH_FLAG_EOP |
							FLASH_FLAG_WRPERR | 
							FLASH_FLAG_PGAERR |
							FLASH_FLAG_SIZERR | 
							FLASH_FLAG_OPTVERR | 
							FLASH_FLAG_OPTVERRUSR
							);
		}
	}

	return ft;
}

void internalFlashClearFlag(void) {
	/* Clear all pending flags */
	FLASH_ClearFlag(FLASH_FLAG_EOP |
							FLASH_FLAG_WRPERR | 
							FLASH_FLAG_PGAERR |
							FLASH_FLAG_SIZERR | 
							FLASH_FLAG_OPTVERR | 
							FLASH_FLAG_OPTVERRUSR
							);
}

/*---------------------------------------------------------------------------*
 *  DECLARE: EEPROM function
 *  Note:
 *---------------------------------------------------------------------------*/
#define EEPROM_BASE_ADDR	        ( 0x08080000 )
#define EEPROM_BYTE_SIZE	        ( 0x1000 ) /* 4K Bytes */

#define EEPROM_LOCK()       		DATA_EEPROM_Lock()
#define EEPROM_UNLOCK()     		DATA_EEPROM_Unlock()

uint8_t EEPROM_Write(uint32_t biasAddr, uint8_t *buf, uint16_t len) {
	uint16_t id = 0;
    FLASH_Status ft = 0;

    EEPROM_UNLOCK();
	for(id = 0; id < len; id += sizeof(uint8_t)) {
		ft = DATA_EEPROM_ProgramByte(EEPROM_BASE_ADDR + biasAddr + id, 
									*(buf)
									);

		if (ft == FLASH_COMPLETE) {
			++buf;
		}
		else {
			FLASH_ClearFlag(FLASH_FLAG_EOP | 
							FLASH_FLAG_WRPERR | 
							FLASH_FLAG_PGAERR |
							FLASH_FLAG_SIZERR | 
							FLASH_FLAG_OPTVERR
							);
		}
	}
    EEPROM_LOCK();

    return (ft);
}

void EEPROM_Read(uint32_t biasAddr, uint8_t *buf, uint16_t len) {
	uint8_t *_buf;

	_buf = (uint8_t *)((uint32_t *)(EEPROM_BASE_ADDR + biasAddr));

    while(len-- > 0) {
        *(buf++) = *(_buf++);
    }
}

void EEPROM_Clear(uint32_t biasAddr, uint16_t len) {
	uint16_t id = 0;
	FLASH_Status ft = 0;

    EEPROM_UNLOCK();

	while (id < len) {
		watchdogRst();
		ft = DATA_EEPROM_ProgramByte(EEPROM_BASE_ADDR + biasAddr + id, 0x00);
		if (ft == FLASH_COMPLETE) {
			++id;
		}
		else {
			FLASH_ClearFlag(FLASH_FLAG_EOP | 
							FLASH_FLAG_WRPERR | 
							FLASH_FLAG_PGAERR |
							FLASH_FLAG_SIZERR | 
							FLASH_FLAG_OPTVERR
							);
		}
	}

    EEPROM_LOCK();
}

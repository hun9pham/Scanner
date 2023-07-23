#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "io_cfg.h"
#include "sys_ctl.h"
#include "platform.h"

ringBufferChar_t MPUInterfaceRx;
static uint8_t MPUInterfaceRxBuf[UART_MPU_IF_BUFFER_SIZE];

volatile bool MOTOR1_EncoderPulse = false;
volatile bool MOTOR2_EncoderPulse = false;

volatile bool SENSOR2_OutOfPaper = false;
volatile bool SENSOR1_DectecShortPaper = false;

volatile uint8_t EXTI5_EnableDetectOutOfPaper = false;
volatile uint8_t EXTI6_EnableDetectFirstStopScroll = false;
volatile uint8_t EXTI4_EnableDetectShortPaper = false;

void ledLifeInit() {
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = IO_LED_LIFE_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(IO_LED_LIFE_PORT, &GPIO_InitStructure);
}

void ledFlashInit() {
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(IO_LED_FLASH_CLOCK, ENABLE);
	GPIO_InitStructure.GPIO_Pin = IO_LED_FLASH_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(IO_LED_FLASH_PORT, &GPIO_InitStructure);
}

void ledStatusInit() {
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(IO_LED_STATUS_CLOCK, ENABLE);
	GPIO_InitStructure.GPIO_Pin = IO_LED_STATUS_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(IO_LED_STATUS_PORT, &GPIO_InitStructure);
}

void ledDirInit() {
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(IO_LED_DIR_CLOCK, ENABLE);
	GPIO_InitStructure.GPIO_Pin = IO_LED_DIR_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(IO_LED_DIR_PORT, &GPIO_InitStructure);
}

void ledFaultInit() {
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(IO_LED_FAULT_CLOCK, ENABLE);
	GPIO_InitStructure.GPIO_Pin = IO_LED_FAULT_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(IO_LED_FAULT_PORT, &GPIO_InitStructure);
}

void ledLifeOn() {
	GPIO_SetBits(IO_LED_LIFE_PORT, IO_LED_LIFE_PIN);
}

void ledLifeOff() {
	GPIO_ResetBits(IO_LED_LIFE_PORT, IO_LED_LIFE_PIN);
}

void ledLifeToggle() {
	static uint8_t InvertStatus = 1;
	GPIO_WriteBit(IO_LED_LIFE_PORT, IO_LED_LIFE_PIN, InvertStatus);
	InvertStatus = !InvertStatus;
}

void ledFlashOn() {
	GPIO_SetBits(IO_LED_FLASH_PORT, IO_LED_FLASH_PIN);
}

void ledFlashOff() {
	GPIO_ResetBits(IO_LED_FLASH_PORT, IO_LED_FLASH_PIN);
}

void ledFlashToggle() {
	static uint8_t InvertStatus = 1;
	GPIO_WriteBit(IO_LED_FLASH_PORT, IO_LED_FLASH_PIN, InvertStatus);
	InvertStatus = !InvertStatus;
}

void ledStatusOn() {
	GPIO_SetBits(IO_LED_STATUS_PORT, IO_LED_STATUS_PIN);
}

void ledStatusOff() {
	GPIO_ResetBits(IO_LED_STATUS_PORT, IO_LED_STATUS_PIN);
}

void ledStatusToggle() {
	static uint8_t InvertStatus = 1;
	GPIO_WriteBit(IO_LED_STATUS_PORT, IO_LED_STATUS_PIN, InvertStatus);
	InvertStatus = !InvertStatus;
}

void ledDirOn() {
	GPIO_SetBits(IO_LED_DIR_PORT, IO_LED_DIR_PIN);
}

void ledDirOff() {
	GPIO_ResetBits(IO_LED_DIR_PORT, IO_LED_DIR_PIN);
}

void ledDirToggle() {
	static uint8_t InvertStatus = 1;
	GPIO_WriteBit(IO_LED_DIR_PORT, IO_LED_DIR_PIN, InvertStatus);
	InvertStatus = !InvertStatus;
}

void ledFaultOn() {
	GPIO_SetBits(IO_LED_FAULT_PORT, IO_LED_FAULT_PIN);
}

void ledFaultOff() {
	GPIO_ResetBits(IO_LED_FAULT_PORT, IO_LED_FAULT_PIN);
}

void ledFaultToggle() {
	static uint8_t InvertStatus = 1;
	GPIO_WriteBit(IO_LED_FAULT_PORT, IO_LED_FAULT_PIN, InvertStatus);
	InvertStatus = !InvertStatus;
}

void buzzerInit() {
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(IO_BUZZER_CLOCK, ENABLE);
	GPIO_InitStructure.GPIO_Pin = IO_BUZZER_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(IO_BUZER_PORT, &GPIO_InitStructure);
	GPIO_ResetBits(IO_BUZER_PORT, IO_BUZZER_PIN);
}

void makeBeepSound(uint16_t timDelay) {
	GPIO_SetBits(IO_BUZER_PORT, IO_BUZZER_PIN);
	delayMillis(timDelay);
	GPIO_ResetBits(IO_BUZER_PORT, IO_BUZZER_PIN);
}

void MPUInterfaceInit(void) {
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(UART_MPU_IF_CLOCK, ENABLE);
	RCC_AHBPeriphClockCmd(IO_MPU_IF_CLOCK, ENABLE);

	GPIO_PinAFConfig(IO_MPU_IF_PORT, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(IO_MPU_IF_PORT, GPIO_PinSource3, GPIO_AF_USART2);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin = IO_MPU_IF_TX_PIN;
	GPIO_Init(IO_MPU_IF_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = IO_MPU_IF_RX_PIN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(IO_MPU_IF_PORT, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = UART_MPU_IF_BAUD;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART_MPU_IF, &USART_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = UART_MPU_IF_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_ClearITPendingBit(UART_MPU_IF,USART_IT_RXNE);
	USART_ITConfig(UART_MPU_IF, USART_IT_RXNE, ENABLE);
	USART_ITConfig(UART_MPU_IF, USART_IT_TXE, DISABLE);
	USART_Cmd(UART_MPU_IF, ENABLE);

	ringBufferCharInit(&MPUInterfaceRx, MPUInterfaceRxBuf, UART_MPU_IF_BUFFER_SIZE);
}

void putMPUSerialIfData(uint8_t data) {
	while (USART_GetFlagStatus(UART_MPU_IF, USART_FLAG_TXE) == RESET);
	USART_SendData(UART_MPU_IF, (uint8_t)data);
	while (USART_GetFlagStatus(UART_MPU_IF, USART_FLAG_TC) == RESET);
}

void putMPUMessage(const char *str) {
	uint8_t len = strlen(str);
	for (uint8_t id = 0; id < len; ++id) {
		putMPUSerialIfData(str[id]);
	}
}

void inputsInit() {
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHBPeriphClockCmd(IO_INPUTn_CLOCK, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	GPIO_InitStructure.GPIO_Pin = IO_INPUT1_PIN | IO_INPUT2_PIN | IO_INPUT3_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(IO_INPUTn_PORT, &GPIO_InitStructure);

	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource4);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource5);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource6);

	/* Common EXTI Configuration */
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;

	/* Trigger via RISING */
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_Line = EXTI_Line4;
	EXTI_Init(&EXTI_InitStructure);
	EXTI_InitStructure.EXTI_Line = EXTI_Line5;
	EXTI_Init(&EXTI_InitStructure);

	/* Trigger via FALLING */
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_Line = EXTI_Line6;
	EXTI_Init(&EXTI_InitStructure);

	/* Common NVIC Configuration */
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0xFF;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_Init(&NVIC_InitStructure);
	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
	NVIC_Init(&NVIC_InitStructure);
}

void enableEXTI(volatile uint8_t *EXTI_Enable) {
	ENTRY_CRITICAL();
	*EXTI_Enable = 1;
	EXIT_CRITICAL();
}

void disableEXTI(volatile uint8_t *EXTI_Enable) {
	ENTRY_CRITICAL();
	*EXTI_Enable = 0;
	EXIT_CRITICAL();
}

uint8_t readInput1() {
	return GPIO_ReadInputDataBit(IO_INPUTn_PORT, IO_INPUT1_PIN);
}

uint8_t readInput2() {
	return GPIO_ReadInputDataBit(IO_INPUTn_PORT, IO_INPUT2_PIN);
}

uint8_t readInput3() {
	return GPIO_ReadInputDataBit(IO_INPUTn_PORT, IO_INPUT3_PIN);
}

void MOTORS_CtlInit() {
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(IO_MOTOR_PWM_CLOCK, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	
	/* MOTOR 1 */
	GPIO_InitStructure.GPIO_Pin = IO_MOTOR_PWM1L_PIN | IO_MOTOR_PWM1R_PIN;
	GPIO_Init(IO_MOTOR_PWM_PORT, &GPIO_InitStructure);
	/* MOTOR 2 */
	GPIO_InitStructure.GPIO_Pin = IO_MOTOR_PWM2R_PIN | IO_MOTOR_PWM2L_PIN;
	GPIO_Init(IO_MOTOR_PWM_PORT, &GPIO_InitStructure);

	GPIO_ResetBits(IO_MOTOR_PWM_PORT, IO_MOTOR_PWM1R_PIN);
	GPIO_ResetBits(IO_MOTOR_PWM_PORT, IO_MOTOR_PWM2L_PIN);
	GPIO_ResetBits(IO_MOTOR_PWM_PORT, IO_MOTOR_PWM1L_PIN);
	GPIO_ResetBits(IO_MOTOR_PWM_PORT, IO_MOTOR_PWM2R_PIN);
}

void MOTOR1_SetPWM(uint8_t stateCtl) {
	switch (stateCtl) {
	case STOP: {
		GPIO_ResetBits(IO_MOTOR_PWM_PORT, IO_MOTOR_PWM1R_PIN);
		GPIO_ResetBits(IO_MOTOR_PWM_PORT, IO_MOTOR_PWM1L_PIN);
	}
	break;

	case SCROLLFORWARD: {
		GPIO_SetBits(IO_MOTOR_PWM_PORT, IO_MOTOR_PWM1R_PIN);
		GPIO_ResetBits(IO_MOTOR_PWM_PORT, IO_MOTOR_PWM1L_PIN);
	}
	break;

	case SCROLLBACKWARD: {
		GPIO_ResetBits(IO_MOTOR_PWM_PORT, IO_MOTOR_PWM1R_PIN);
		GPIO_SetBits(IO_MOTOR_PWM_PORT, IO_MOTOR_PWM1L_PIN);
	}
	break;
	
	default:
	break;
	}
}

void MOTOR2_SetPWM(uint8_t stateCtl) {
	switch (stateCtl) {
	case STOP: {
		GPIO_ResetBits(IO_MOTOR_PWM_PORT, IO_MOTOR_PWM2R_PIN);
		GPIO_ResetBits(IO_MOTOR_PWM_PORT, IO_MOTOR_PWM2L_PIN);
	}
	break;

	case SCROLLFORWARD: {
		GPIO_ResetBits(IO_MOTOR_PWM_PORT, IO_MOTOR_PWM2R_PIN);
		GPIO_SetBits(IO_MOTOR_PWM_PORT, IO_MOTOR_PWM2L_PIN);
	}
	break;

	case SCROLLBACKWARD: {
		GPIO_SetBits(IO_MOTOR_PWM_PORT, IO_MOTOR_PWM2R_PIN);
		GPIO_ResetBits(IO_MOTOR_PWM_PORT, IO_MOTOR_PWM2L_PIN);
	}
	break;
	
	default:
	break;
	}
}

void MOTOR1_EncoderPinoutInit() {
	TIM_ICInitTypeDef TIM_ICInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
  	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	RCC_AHBPeriphClockCmd(IO_MOTOR_PWM_CLOCK, ENABLE);

	/* TIM3_CH3 as Input capture */
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin   = IO_MOTOR_C2B1_PIN;
	GPIO_Init(IO_MOTOR_PWM_PORT, &GPIO_InitStructure);
	GPIO_PinAFConfig(IO_MOTOR_PWM_PORT, GPIO_PinSource0, GPIO_AF_TIM3);

	TIM_ICInitStructure.TIM_Channel     = TIM_Channel_3;
	TIM_ICInitStructure.TIM_ICPolarity  = TIM_ICPolarity_Falling;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0x0;
	TIM_ICInit(TIM3, &TIM_ICInitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0xFF;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0xFF;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure); 

	TIM_ITConfig(TIM3, TIM_IT_CC3, ENABLE);
	TIM_Cmd(TIM3, ENABLE);
}

void MOTOR2_EncoderPinoutInit() {
	TIM_ICInitTypeDef TIM_ICInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
  	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	RCC_AHBPeriphClockCmd(IO_MOTOR_PWM_CLOCK, ENABLE);

	/* TIM2_CH4 as Input capture */
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin   = IO_MOTOR_C2B2_PIN;
	GPIO_Init(IO_MOTOR_PWM_PORT, &GPIO_InitStructure);
	GPIO_PinAFConfig(IO_MOTOR_PWM_PORT, GPIO_PinSource11, GPIO_AF_TIM2);

	TIM_ICInitStructure.TIM_Channel     = TIM_Channel_4;
	TIM_ICInitStructure.TIM_ICPolarity  = TIM_ICPolarity_Falling;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0x0;
	TIM_ICInit(TIM2, &TIM_ICInitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0xFF;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0xFF;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure); 
	
	TIM_ITConfig(TIM2, TIM_IT_CC4, ENABLE);
	TIM_Cmd(TIM2, ENABLE);
}

bool MOTOR1_IsRun() {
	ENTRY_CRITICAL();
	bool ret = MOTOR1_EncoderPulse;
	MOTOR1_EncoderPulse = false;
	EXIT_CRITICAL();
	return ret;
}

bool MOTOR2_IsRun() {
	ENTRY_CRITICAL();
	bool ret = MOTOR2_EncoderPulse;
	MOTOR2_EncoderPulse = false;
	EXIT_CRITICAL();
	return ret;
}
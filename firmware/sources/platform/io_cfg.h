#ifndef __IO_CFG_H
#define __IO_CFG_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#include "ring_buffer.h"

#include "stm32xx_header.h"

/*----------------------------------------------------------------------------*
 *  DECLARE: All LEDs pin map definitions
 *  Note: 
 *----------------------------------------------------------------------------*/
#define IO_LED_LIFE_CLOCK                       ( RCC_AHBPeriph_GPIOB )
#define IO_LED_LIFE_PIN				            ( GPIO_Pin_9 )
#define IO_LED_LIFE_PORT				        ( GPIOB )

#define IO_LED_FLASH_CLOCK                      ( RCC_AHBPeriph_GPIOA )
#define IO_LED_FLASH_PIN				        ( GPIO_Pin_7 )
#define IO_LED_FLASH_PORT				        ( GPIOA )

#define IO_LED_STATUS_CLOCK                     ( RCC_AHBPeriph_GPIOA )
#define IO_LED_STATUS_PIN				        ( GPIO_Pin_8 )
#define IO_LED_STATUS_PORT				        ( GPIOA )

#define IO_LED_DIR_CLOCK                        ( RCC_AHBPeriph_GPIOB )
#define IO_LED_DIR_PIN				            ( GPIO_Pin_11 )
#define IO_LED_DIR_PORT				            ( GPIOA )

#define IO_LED_FAULT_CLOCK                      ( RCC_AHBPeriph_GPIOA )
#define IO_LED_FAULT_PIN				        ( GPIO_Pin_12 )
#define IO_LED_FAULT_PORT				        ( GPIOA )

/*----------------------------------------------------------------------------*
 *  DECLARE: Buzzer
 *  Note: 
 *----------------------------------------------------------------------------*/
#define IO_BUZZER_CLOCK                         ( RCC_AHBPeriph_GPIOA )
#define IO_BUZZER_PIN				            ( GPIO_Pin_1 )
#define IO_BUZER_PORT   				        ( GPIOA )

/*----------------------------------------------------------------------------*
 *  DECLARE: MPU Serial interface
 *  Note: Communication with PC, Pi, ...
 *----------------------------------------------------------------------------*/
#define UART_MPU_IF_CLOCK                       ( RCC_APB1Periph_USART2 )
#define UART_MPU_IF                             ( USART2 )
#define UART_MPU_IF_IRQn                        ( USART2_IRQn )
#define UART_MPU_IF_BAUD                        ( 115200 )

#define IO_MPU_IF_CLOCK                         ( RCC_AHBPeriph_GPIOA )
#define IO_MPU_IF_PORT                          ( GPIOA )
#define IO_MPU_IF_TX_PIN                        ( GPIO_Pin_2 )
#define IO_MPU_IF_RX_PIN                        ( GPIO_Pin_3 )

#define UART_MPU_IF_BUFFER_SIZE                 ( 128 )

/*----------------------------------------------------------------------------*
 *  DECLARE: Input[1:3] pin map definitions
 *  Note: 
 *----------------------------------------------------------------------------*/
#define IO_INPUTn_CLOCK                         ( RCC_AHBPeriph_GPIOA )
#define IO_INPUT1_PIN                           ( GPIO_Pin_4 )
#define IO_INPUT2_PIN                           ( GPIO_Pin_5 )
#define IO_INPUT3_PIN                           ( GPIO_Pin_6 )
#define IO_INPUTn_PORT                          ( GPIOA )

/*----------------------------------------------------------------------------*
 *  DECLARE: Motors PWM
 *  Note: 
 *----------------------------------------------------------------------------*/
#define IO_MOTOR_PWM_CLOCK                      ( RCC_AHBPeriph_GPIOB )
#define IO_MOTOR_PWM_PORT                       ( GPIOB )
/* Engine 1 */
#define IO_MOTOR_PWM1R_PIN                      ( GPIO_Pin_15 ) /* TIM11_CH1 */
#define IO_MOTOR_PWM1L_PIN                      ( GPIO_Pin_14 ) /* TIM9_CH2 */
#define IO_MOTOR_C2B1_PIN                       ( GPIO_Pin_0 ) /* TIM3_CH3 */
#define IO_MOTOR_C1A1_PIN                       ( GPIO_Pin_1 ) /* TIM3_CH4 */
/* Engine 2 */
#define IO_MOTOR_PWM2R_PIN                      ( GPIO_Pin_13 ) /* TIM9_CH1 */
#define IO_MOTOR_PWM2L_PIN                      ( GPIO_Pin_12 ) /* TIM10_CH1 */
#define IO_MOTOR_C2B2_PIN                       ( GPIO_Pin_11 ) /* TIM2_CH4 */
#define IO_MOTOR_C1A2_PIN                       ( GPIO_Pin_10 ) /* TIM2_CH3 */

/* Enumarics -----------------------------------------------------------------*/

/* Extern variables ----------------------------------------------------------*/
extern ringBufferChar_t MPUInterfaceRx;

extern volatile bool MOTOR1_EncoderInput;
extern volatile bool MOTOR2_EncoderInput;

/* Function prototypes -------------------------------------------------------*/
extern void ledLifeInit();
extern void ledFlashInit();
extern void ledStatusInit();
extern void ledDirInit();
extern void ledFaultInit();
extern void buzzerInit();
extern void MPUInterfaceInit();
extern void inputsInit();
extern void MOTORS_PWMInit(); /* PWM Control */
extern void MOTOR1_EncoderPinoutInit();
extern void MOTOR2_EncoderPinoutInit();

extern void ledLifeOn();
extern void ledLifeOff();
extern void ledLifeToggle();
extern void ledFlashOn();
extern void ledFlashOff();
extern void ledFlashToggle();
extern void ledStatusOn();
extern void ledStatusOff();
extern void ledStatusToggle();
extern void ledDirOn();
extern void ledDirOff();
extern void ledDirToggle();
extern void ledFaultOn();
extern void ledFaultOff();
extern void ledFaultToggle();
extern void makeBeepSound(uint16_t timDelay);
extern uint8_t readInput1();
extern uint8_t readInput2();
extern uint8_t readInput3();
extern uint8_t getMPUSerialIfData();
extern void putMPUSerialIfData(uint8_t ch);
extern void putMPUMessage(const char *str);
extern void MOTOR1_SetPWM(uint8_t percent);
extern void MOTOR2_SetPWM(uint8_t percent);
extern bool MOTOR1_IsRun();
extern bool MOTOR2_IsRun();

#define readSensor1()     readInput1() /* Cảm biến số 1 (Ngoài cùng) */
#define readSensor2()     readInput2() /* Cảm biến số 2 (Ngoài cùng) */
#define readSensor3()     readInput3() /* Cảm biến số 3 (Ở giữa) */

#ifdef __cplusplus
}
#endif

#endif /* __IO_CFG_H */

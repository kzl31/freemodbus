#ifndef _PORT_INTERNAL_H
#define _PORT_INTERNAL_H

#include "port.h"

/* 检测 STM32 系列并包含相应的 HAL 头文件 */
#if defined(STM32F3xx)
  #include "stm32f3xx_hal.h"
#elif defined(STM32G4xx) || defined(STM32G431xx)
  #include "stm32g4xx_hal.h"
#elif defined(STM32F1xx)
  #include "stm32f1xx_hal.h"
/* 根据需要添加其他 STM32 系列 */
#else
  #error "未定义 STM32 设备系列！"
#endif

/* 如果未定义 USART IRQ 优先级和子优先级，则定义默认值 */
#ifndef MB_USART_IRQ_priority
  #define MB_USART_IRQ_priority     3
#endif
#ifndef MB_USART_IRQ_subpriority
  #define MB_USART_IRQ_subpriority  1
#endif

/* 如果未定义 TIM7 IRQ 优先级和子优先级，则定义默认值 */
#ifndef MB_TIM7_IRQ_priority
  #define MB_TIM7_IRQ_priority     4
#endif
#ifndef MB_TIM7_IRQ_subpriority
  #define MB_TIM7_IRQ_subpriority  1
#endif

/* 如果未定义 MB_USART_NR，则默认使用 USART2 */
#ifndef MB_USART_NR
  #define MB_USART_NR   2
#endif

/* 根据选择的 USART 定义宏 */
#if (MB_USART_NR == 1)

  #define MB_USART                        USART1
  #define MB_USART_IRQn                   USART1_IRQn
  #define MB_USART_IRQHandler             USART1_IRQHandler
  #define MB_USART_CLK_ENABLE()           __HAL_RCC_USART1_CLK_ENABLE()
  #define MB_USART_CLK_DISABLE()          __HAL_RCC_USART1_CLK_DISABLE()
  #define MB_TX_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOA_CLK_ENABLE()
  #define MB_TX_GPIO_CLK_DISABLE()        __HAL_RCC_GPIOA_CLK_DISABLE()
  #define MB_TX_AF                        GPIO_AF7_USART1
  #define MB_RX_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOA_CLK_ENABLE()
  #define MB_RX_GPIO_CLK_DISABLE()        __HAL_RCC_GPIOA_CLK_DISABLE()
  #define MB_RX_AF                        GPIO_AF7_USART1

#elif (MB_USART_NR == 2)

  #define MB_USART                        USART2
  #define MB_USART_IRQn                   USART2_IRQn
  #define MB_USART_IRQHandler             USART2_IRQHandler
  #define MB_USART_CLK_ENABLE()           __HAL_RCC_USART2_CLK_ENABLE()
  #define MB_USART_CLK_DISABLE()          __HAL_RCC_USART2_CLK_DISABLE()
  #define MB_TX_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOA_CLK_ENABLE()
  #define MB_TX_GPIO_CLK_DISABLE()        __HAL_RCC_GPIOA_CLK_DISABLE()
  #define MB_TX_AF                        GPIO_AF7_USART2
  #define MB_RX_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOA_CLK_ENABLE()
  #define MB_RX_GPIO_CLK_DISABLE()        __HAL_RCC_GPIOA_CLK_DISABLE()
  #define MB_RX_AF                        GPIO_AF7_USART2

#else
  #error "不支持 MB_USART 配置。请将 MB_USART_NR 定义为 1 或 2。"
#endif

//#define MB_TIMER_DEBUG              1
#define MB_TIMER_DEBUG_PORT         GPIOA
#define MB_TIMER_DEBUG_PIN          GPIO_PIN_5  /* PA5 连接到大多数 Nucleo 板上的板载 LED */

/* 调试辅助函数 */
#if MB_TIMER_DEBUG == 1
static inline void vMBTimerDebugSetHigh( void )
{
    HAL_GPIO_WritePin( MB_TIMER_DEBUG_PORT, MB_TIMER_DEBUG_PIN, GPIO_PIN_SET );
}

static inline void vMBTimerDebugSetLow( void )
{
    HAL_GPIO_WritePin( MB_TIMER_DEBUG_PORT, MB_TIMER_DEBUG_PIN, GPIO_PIN_RESET );
}
#else
#define vMBTimerDebugSetHigh()
#define vMBTimerDebugSetLow()
#endif


void                    MB_Uart_Init(void);

#endif /* _PORT_INTERNAL_H */
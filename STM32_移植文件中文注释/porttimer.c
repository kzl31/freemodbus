#include "mb.h"
#include "mbport.h"
#include "port_internal.h"
#include "main.h"  // 包含 GPIO 定义

static TIM_HandleTypeDef h_tim7;

/* 静态变量 */
static USHORT           timeout     = 0;
static USHORT           downcounter = 0;

BOOL xMBPortTimersInit( USHORT usTim1Timerout50us )
{
    __HAL_RCC_TIM7_CLK_ENABLE();

    TIM_MasterConfigTypeDef sMasterConfig;
    
    h_tim7.Instance = TIM7;
    h_tim7.Init.Prescaler = (HAL_RCC_GetPCLK1Freq() / 1000000) - 1;
    h_tim7.Init.CounterMode = TIM_COUNTERMODE_UP;
    h_tim7.Init.Period = 50 - 1;

    timeout = usTim1Timerout50us;

    if (HAL_TIM_Base_Init(&h_tim7) != HAL_OK)
    {
    return FALSE;
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&h_tim7, &sMasterConfig) != HAL_OK)
    {
    return FALSE;
    }

    /* 为定时器中断配置 NVIC */
    HAL_NVIC_SetPriority(TIM7_IRQn, MB_TIM7_IRQ_priority, MB_TIM7_IRQ_subpriority);
    
    /* 设置调试输出引脚 */
    #if MB_TIMER_DEBUG == 1
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* 配置调试 GPIO 引脚 */
    GPIO_InitStruct.Pin = MB_TIMER_DEBUG_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(MB_TIMER_DEBUG_PORT, &GPIO_InitStruct);
    
    /* 初始状态：低电平 */
    vMBTimerDebugSetLow();
    #endif

    return TRUE;
}

void vMBPortTimersEnable( void )
{
  /* 启用定时器，使用传递给 xMBPortTimersInit() 的超时值 */
  downcounter = timeout;
  
  /* 设置调试引脚高电平以指示定时器激活 */
  vMBTimerDebugSetHigh();
  
  HAL_TIM_Base_Start_IT(&h_tim7);
  HAL_NVIC_EnableIRQ(TIM7_IRQn);
}

void vMBPortTimersDisable( void )
{
  /* 禁用任何待处理的定时器。 */
  HAL_TIM_Base_Stop_IT(&h_tim7);
  HAL_NVIC_DisableIRQ(TIM7_IRQn);
  
  /* 设置调试引脚低电平以指示定时器已禁用 */
  vMBTimerDebugSetLow();
}

/**
 * @brief 此函数处理 TIM7 全局中断。
 */
void TIM7_IRQHandler( void )
{
    /* 检查更新中断标志是否设置 */
    if(__HAL_TIM_GET_FLAG(&h_tim7, TIM_FLAG_UPDATE) != RESET && 
       __HAL_TIM_GET_IT_SOURCE(&h_tim7, TIM_IT_UPDATE) != RESET) {
        /* 清除更新中断标志 */
        __HAL_TIM_CLEAR_IT(&h_tim7, TIM_IT_UPDATE);

        /* 递减计数器并检查是否达到零 */
        if (--downcounter == 0) {
            /* 定时器到期，调用回调函数 */
            vMBTimerDebugSetLow();
            pxMBPortCBTimerExpired();
        }
    }

    /* 调用 HAL 定时器 IRQ 处理函数 */
    HAL_TIM_IRQHandler(&h_tim7);
}
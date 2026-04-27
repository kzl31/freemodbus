#include "mb.h"
#include "mbport.h"
#include "port_internal.h"

UART_HandleTypeDef          uart_mb; 


/* 注意 ST 处理器的 UART 配置与通用配置不同：

    https://community.st.com/t5/stm32-mcus-products/uart-parity-and-data-bit-issue-in-stm32c0-series/td-p/713896

    通用配置：8E1（8数据位+偶校验+1停止位）

    ST 配置：
    UartHandle.Init.WordLength = UART_WORDLENGTH_9B; // 8数据位+奇偶校验
    UartHandle.Init.StopBits   = UART_STOPBITS_1;
    UartHandle.Init.Parity     = UART_PARITY_EVEN;

    通用配置：7E1（7数据位+偶校验+1停止位）

    ST 配置：
    UartHandle.Init.WordLength = UART_WORDLENGTH_8B; // 7数据位+奇偶校验
    UartHandle.Init.StopBits   = UART_STOPBITS_1;
    UartHandle.Init.Parity     = UART_PARITY_EVEN;
*/


BOOL xMBPortSerialInit( UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{
    UNUSED( ucPORT );

    // 配置用于 Modbus 通信的 UART
    uart_mb.Instance          = MB_PARITY;
    uart_mb.Init.BaudRate     = ulBaudRate;
    uart_mb.Init.StopBits     = UART_STOPBITS_1; // 始终使用 1 个停止位
    uart_mb.Init.Mode         = UART_MODE_TX_RX;
    uart_mb.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    uart_mb.Init.OverSampling = UART_OVERSAMPLING_16;

    // 根据数据位和奇偶校验配置字长和奇偶校验
    if( ucDataBits == 8 )
    {
        if( eParity == MB_PAR_NONE )
        {
            uart_mb.Init.WordLength = UART_WORDLENGTH_8B;
            uart_mb.Init.Parity     = UART_PARITY_NONE;
        }
        else
        {
            uart_mb.Init.WordLength = UART_WORDLENGTH_9B; // 8数据位+奇偶校验
            uart_mb.Init.Parity     = (eParity == MB_PAR_ODD) ? UART_PARITY_ODD : UART_PARITY_EVEN;
        }
    }
    else if( ucDataBits == 7 )
    {
        if( eParity == MB_PAR_NONE )
        {
            uart_mb.Init.WordLength = UART_WORDLENGTH_7B;
            uart_mb.Init.Parity     = UART_PARITY_NONE;
        }
        else
        {
            uart_mb.Init.WordLength = UART_WORDLENGTH_8B; // 7数据位+奇偶校验
            uart_mb.Init.Parity     = (eParity == MB_PAR_ODD) ? UART_PARITY_ODD : UART_PARITY_EVEN;
        }
    }
    else
    {
        return FALSE; // 不支持的数据位配置
    }

    if( HAL_UART_Init( &uart_mb ) != HAL_OK )
    {
        return FALSE; // UART 初始化失败
    }

    // 初始时禁用 RX 和 TX 中断
    __HAL_UART_DISABLE_IT(&uart_mb, UART_IT_RXNE);
    __HAL_UART_DISABLE_IT(&uart_mb, UART_IT_TXE);

    return TRUE;
}

void MB_Uart_Init(void)
{
    // 启用 UART 外设时钟
    MB_USART_CLK_ENABLE();

    // 启用 GPIO 时钟
    MB_TX_GPIO_CLK_ENABLE();
    MB_RX_GPIO_CLK_ENABLE();

    // 为 UART 中断配置 NVIC
    HAL_NVIC_SetPriority(MB_USART_IRQn, MB_USART_IRQ_priority, MB_USART_IRQ_subpriority);
    HAL_NVIC_DisableIRQ(MB_USART_IRCOM端口 IRQn);
}

void vMBPortSerialEnable(BOOL rxEnable, BOOL txEnable)
{
    // 在配置期间禁用中断
    HAL_NVIC_DisableIRQ(MB_USART_IRQn);
    
    // 配置接收中断
    if( rxEnable )
        MB_USART->CR1 |= USART_CR1_RXNEIE;
    else
        MB_USART->CR1 &= ~USART_CR1_RXNEIE;

    // 配置发送中断
    if( txEnable )
        MB_USART->CR1 |= USART_CR1_TXEIE;
    else
        MB_USART->CR1 &= ~USART_CR1_TXEIE;

    // 仅当至少一个方向激活时重新启用 UART 中断
    if( rxEnable || txEnable )
        HAL_NVIC_EnableIRQ(MB_USART_IRQn);
}

BOOL xMBPortSerialPutByte(CHAR byte)
{
    MB_USART->TDR = byte;
    return TRUE;
}

BOOL xMBPortSerialGetByte(CHAR *byte)
{
    *byte = MB_USART->RDR;
    return TRUE;
}

void MB_USART_IRQHandler(void)
{
    uint32_t isr = MB_USART->ISR;
    uint32_t cr1 = MB_USART->CR1;

    // 检查接收中断
    if( (isr & USART_ISR_RXNE) && (cr1 & USART_CR1_RXNEIE) )
    {
        vMBTimerDebugSetLow();
        pxMBFrameCBByteReceived();
    }

    // 检查发送中断
    if( (isr & USART_ISR_TXE) && (cr1 & USART_CR1_TXEIE) )
    {
        pxMBFrameCBTransmitterEmpty();
    }
    
    // 清除错误标志 - 使用 USART_ICR_NECF 而不是 USART_ICR_NCF
    MB_USART->ICR = (USART_ICR_PECF | USART_ICR_FECF | USART_ICR_NECF | 
                    USART_ICR_ORECF | USART_ICR_IDLECF);
                    
    /* 注意：不要在此处调用 HAL_UART_IRQHandler，因为它会干扰我们的直接寄存器访问 */
}

/* ----------------------- 文件结束 --------------------------------------*/
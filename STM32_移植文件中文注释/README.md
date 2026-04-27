# FreeModbus STM32 移植文件中文注释版

## 概述

此目录包含 FreeModbus 协议栈在 STM32 微控制器上移植所需的所有关键文件，已将其中的英文注释翻译为中文，方便中文开发者使用。

## 文件列表

### 1. 核心移植文件

1. **port.h** - 主端口配置文件
   - 数据类型定义（BOOL, UCHAR, CHAR, USHORT, SHORT, ULONG, LONG）
   - 临界区操作宏定义
   - CMSIS 编译器头文件包含

2. **port_internal.h** - 内部端口配置
   - STM32 系列检测与 HAL 头文件包含
   - UART/TIM7 中断优先级配置
   - USART 选择宏定义
   - 调试引脚配置

3. **portserial.c** - UART/串口通信实现
   - ST 处理器 UART 配置特殊性说明（8E1/7E1）
   - UART 初始化、中断管理
   - 字节收发函数

4. **porttimer.c** - 定时器实现
   - TIM7 定时器配置（50μs 基础周期）
   - 定时器中断处理
   - 调试引脚控制

5. **portevent.c** - 事件处理
   - 简单事件队列实现

### 2. 硬件接口配置

#### UART 配置
```c
// ST 处理器 UART 配置特殊性：
// 通用 8E1 -> ST: UART_WORDLENGTH_9B (8数据位+奇偶校验)
// 通用 7E1 -> ST: UART_WORDLENGTH_8B (7数据位+奇偶校验)
```

#### 定时器配置
```c
// TIM7 配置：
// 预分频器：1MHz 时钟
// 计数器模式：向上计数
// 周期：50-1（50μs 中断周期）
```

#### 中断优先级
```c
// 默认优先级：
// UART: 优先级 3，子优先级 1
// TIM7: 优先级 4，子优先级 1
```

### 3. 关键注意事项

#### STM32 UART 特殊性
1. **数据位+奇偶校验的组合**：
   - 8数据位+奇偶校验 → 需要配置为 9 位字长
   - 7数据位+奇偶校验 → 需要配置为 8 位字长

2. **中断处理**：
   - 不要调用 HAL_UART_IRQHandler()，因为直接寄存器访问
   - 使用 USART_ICR_NECF 清除噪声错误标志

#### 定时器调试
- 调试引脚：PA5（Nucleo 板上的 LED）
- 定时器启用：引脚高电平
- 定时器禁用：引脚低电平
- 中断触发：短暂低电平脉冲

### 4. 移植步骤概要

1. **选择 STM32 系列**：
   - 在 `port_internal.h` 中设置正确的 STM32 宏定义
   - 包含相应的 HAL 头文件

2. **配置 UART**：
   - 设置 `MB_USART_NR`（1 或 2）
   - 确保 GPIO 引脚配置匹配硬件

3. **配置定时器**：
   - TIM7 在许多 STM32 系列中可用
   - 如有需要，可修改为其他定时器

4. **集成到项目**：
   - 将移植文件添加到项目
   - 配置 CMake 或 Makefile
   - 实现寄存器回调函数

### 5. 示例用法

```c
// 初始化 Modbus
void modbusInit(void) {
    MB_Uart_Init();
    eMBInit(MB_RTU, SLAVE_ID, 0, 115200, MB_PAR_NONE);
    eMBEnable();
}

// 主循环中调用
void main(void) {
    modbusInit();
    while(1) {
        eMBPoll();
        // 应用程序代码
    }
}
```

### 6. 许可证

- 端口文件：LGPL 许可证
- 可集成到专有软件（满足 LGPL 条件）

### 7. 测试

使用 Modbus 主站工具（如 modpoll）测试：
```bash
modpoll.exe -m rtu -a 10 -r 1000 -c 4 -d 8 -b 115200 -p none COM7
```

## 技术支持

如需进一步帮助，请参考：
1. STM32 HAL 库文档
2. FreeModbus 官方文档
3. STM32CubeMX 用户手册

## 版本历史

- v1.0：初始版本，基于 STM32_CMAKE 示例项目
- 注释翻译：所有关键英文注释已翻译为中文
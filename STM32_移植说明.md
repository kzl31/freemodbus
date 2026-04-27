# FreeModbus STM32 移植说明

## 概述

本文档提供 FreeModbus 协议栈在 STM32 微控制器上的移植指南。基于 STM32_CMAKE 示例项目，涵盖必要的移植文件、硬件接口配置及移植步骤。

## 文件结构

### 核心移植文件

1. **port.h** - 主端口配置文件
   - 定义数据类型（BOOL, UCHAR, CHAR, USHORT, SHORT, ULONG, LONG）
   - 定义临界区操作宏（ENTER_CRITICAL_SECTION, EXIT_CRITICAL_SECTION）
   - 包含 CMSIS 编译器头文件

2. **port_internal.h** - 内部端口配置
   - STM32 系列检测与 HAL 头文件包含
   - UART 中断优先级定义（MB_USART_IRQ_priority, MB_USART_IRQ_subpriority）
   - TIM7 中断优先级定义（MB_TIM7_IRQ_priority, MB_TIM7_IRQ_subpriority）
   - USART 选择宏定义（MB_USART_NR）
   - 定时器调试引脚配置（MB_TIMER_DEBUG_PORT, MB_TIMER_DEBUG_PIN）

3. **portserial.c** - UART/串口通信实现
   - ST 处理器 UART 配置特殊性说明
   - 8E1 和 7E1 配置的特殊处理
   - UART 初始化、中断管理、字节收发函数

4. **porttimer.c** - 定时器实现
   - TIM7 定时器配置（50μs 基础周期）
   - 定时器中断处理
   - 调试引脚控制

5. **portevent.c** - 事件处理
   - 简单事件队列实现

6. **demo.c** - 示例应用
   - Modbus RTU 从站初始化
   - 寄存器回调函数示例
   - 主循环中的 Modbus 轮询

### 硬件接口定义

#### 1. UART 配置（串口通信）
```c
// port_internal.h 中的配置宏
#define MB_USART_NR   2   // 使用 USART2

#if (MB_USART_NR == 1)
  #define MB_USART                        USART1
  #define MB_USART_IRQn                   USART1_IRQn
  #define MB_USART_IRQHandler             USART1_IRQHandler
  #define MB_USART_CLK_ENABLE()           __HAL_RCC_USART1_CLK_ENABLE()
  // GPIO 引脚配置...
#elif (MB_USART_NR == 2)
  #define MB_USART                        USART2
  #define MB_USART_IRQn                   USART2_IRQn
  #define MB_USART_IRQHandler             USART2_IRQHandler
  #define MB_USART_CLK_ENABLE()           __HAL_RCC_USART2_CLK_ENABLE()
  // GPIO 引脚配置...
#endif
```

**关键注释翻译：**
- ST 处理器 UART 配置与通用配置不同，需注意：
  - **通用配置 8E1**：ST 配置应为 `UART_WORDLENGTH_9B`（8数据位+奇偶校验）
  - **通用配置 7E1**：ST 配置应为 `UART_WORDLENGTH_8B`（7数据位+奇偶校验）

#### 2. 定时器配置（TIM7）
```c
// porttimer.c 中的配置
h_tim7.Instance = TIM7;
h_tim7.Init.Prescaler = (HAL_RCC_GetPCLK1Freq() / 1000000) - 1; // 1MHz 时钟
h_tim7.Init.CounterMode = TIM_COUNTERMODE_UP;
h_tim7.Init.Period = 50 - 1; // 50μs 周期
```

**定时器工作原理：**
- 定时器以 50μs 为基本周期中断
- 递减计数器跟踪超时时间
- 当计数器归零时调用回调函数 `pxMBPortCBTimerExpired()`

#### 3. 中断优先级配置
```c
// port_internal.h 中的默认优先级
#define MB_USART_IRQ_priority     3  // UART 中断优先级
#define MB_USART_IRQ_subpriority  1  // UART 中断子优先级
#define MB_TIM7_IRQ_priority     4  // TIM7 中断优先级
#define MB_TIM7_IRQ_subpriority  1  // TIM7 中断子优先级
```

#### 4. 调试引脚配置
```c
// port_internal.h 中的调试配置
#define MB_TIMER_DEBUG_PORT         GPIOA
#define MB_TIMER_DEBUG_PIN          GPIO_PIN_5  // PA5（大多数 Nucleo 板上的 LED）
```

## 移植步骤

### 第 1 步：准备开发环境
1. **安装工具链**：
   - Visual Studio Code + STM32 VS Code 扩展
   - STM32CubeMX
   - ARM GCC 工具链
   - CMake + Ninja（可选）

2. **创建 STM32CubeMX 项目**：
   - 选择 Nucleo 板（如 NUCLEO-G431RB）
   - 激活 **TIM7** 定时器
   - 虚拟 COM 端口（LPUART1/USART2）通常已由 BSP 包启用
   - 选择 "CMake" 作为工具链/IDE

3. **生成代码**：
   - 点击 "Generate Code" 创建项目文件

### 第 2 步：集成 FreeModbus 库
**选项 A：克隆仓库**
```bash
git clone https://github.com/alammertink/freemodbus.git
cp -r freemodbus/demo/STM32_CMAKE/.vscode ./
cp freemodbus/demo/STM32_CMAKE/CMakeLists.txt .
```

**选项 B：Git 子模块**
```bash
git init
git submodule add https://github.com/alammertink/freemodbus.git
cp -r freemodbus/demo/STM32_CMAKE/.vscode ./
cp freemodbus/demo/STM32_CMAKE/CMakeLists.txt .
```

### 第 3 步：CMake 配置
```cmake
# 添加 FreeModbus 库
add_subdirectory(${CMAKE_SOURCE_DIR}/freemodbus ${CMAKE_BINARY_DIR}/freemodbus)

# 添加源文件到可执行文件
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # ...现有源文件...
    ${FREEMODBUS_APP_SOURCES} # FreeModbus 的平台依赖源文件
    ${CMAKE_SOURCE_DIR}/freemodbus/demo/STM32_CMAKE/demo.c
)

# 添加 FreeModbus 包含目录
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # ...现有包含路径...
    ${FREEMODBUS_INCLUDE_DIRS}
)

# 链接 FreeModbus 库
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    # ...现有库...
    freemodbus
)
```

### 第 4 步：配置 STM32 系列
在 `port_internal.h` 中根据您的 STM32 系列进行配置：
```c
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
```

### 第 5 步：硬件特定配置
1. **UART 选择**：
   - 修改 `port_internal.h` 中的 `MB_USART_NR`（1 或 2）
   - 确保 GPIO 引脚配置与您的硬件匹配

2. **定时器配置**：
   - TIM7 在许多 STM32 系列中可用（F1/F2/F3/F4/F7, G0/G4, H7, L0/L4/L5, WB）
   - 如需使用其他定时器，需修改 `porttimer.c`

3. **中断优先级**：
   - 根据系统需求调整中断优先级

### 第 6 步：应用程序集成
1. **初始化 Modbus**：
```c
void modbusInit(void)
{
    eMBErrorCode eStatus;
    
    MB_Uart_Init(); // 初始化 UART 硬件
    
    // 初始化 Modbus 协议栈
    eStatus = eMBInit(MB_RTU, SLAVE_ID, 0, MB_BAUDRATE, MB_PAR_NONE);
    
    // 启用 Modbus 协议栈
    eStatus = eMBEnable();
}
```

2. **主循环轮询**：
```c
void modbusPoll(void)
{
    // 处理 Modbus 事件
    (void)eMBPoll();
    
    // 更新应用程序状态
    // ...
}
```

3. **寄存器回调函数**：
   - 实现输入寄存器、保持寄存器、线圈和离散输入回调函数
   - 参考 `demo.c` 中的示例实现

## 关键注意事项

### 1. STM32 UART 特殊性
**重要注释翻译：**
"注意 ST 处理器的 UART 配置与通用配置不同：
通用配置 8E1（8数据位+偶校验+1停止位）
ST 配置：`UART_WORDLENGTH_9B`（8数据位+奇偶校验），`UART_STOPBITS_1`，`UART_PARITY_EVEN`

通用配置 7E1（7数据位+偶校验+1停止位）
ST 配置：`UART_WORDLENGTH_8B`（7数据位+奇偶校验），`UART_STOPBITS_1`，`UART_PARITY_EVEN`"

### 2. 定时器调试
调试引脚（PA5）可用于监控定时器状态：
- 定时器启用时：引脚置高
- 定时器禁用时：引脚置低
- 定时器中断处理时：短暂置低以指示中断触发

### 3. 中断处理
**关键注释翻译：**
"不要在此处调用 HAL_UART_IRQHandler，因为它会干扰我们的直接寄存器访问"

### 4. 兼容性
- **虚拟 COM 端口**：所有 STM32 Nucleo 板都有连接到 ST-LINK 的虚拟 COM 端口
- **TIM7 支持**：广泛的 STM32 系列支持
- **BSP 包**：利用 STM32 Nucleo BSP 实现跨系列标准化接口

## 测试方法

### 使用 modpoll 测试
```bash
modpoll.exe -m rtu -a 10 -r 1000 -c 4 -d 8 -t 3 -b 115200 -p none -s 1 COM7
```

### 测试参数说明：
- `-m rtu`：使用 Modbus RTU 协议
- `-a 10`：从站地址为 10
- `-r 1000`：从寄存器地址 1000 开始读取
- `-c 4`：读取 4 个寄存器
- `-d 8`：数据位为 8
- `-b 115200`：波特率 115200
- `-p none`：无奇偶校验
- `COM7`：使用的 COM 端口

## 文件清单

### 必需移植文件
1. `demo/STM32_CMAKE/port/port.h`
2. `demo/STM32_CMAKE/port/port_internal.h`
3. `demo/STM32_CMAKE/port/portserial.c`
4. `demo/STM32_CMAKE/port/porttimer.c`
5. `demo/STM32_CMAKE/port/portevent.c`

### 可选示例文件
1. `demo/STM32_CMAKE/demo.c`（示例应用）
2. `demo/STM32_CMAKE/README.md`（英文说明）
3. `demo/STM32_CMAKE/CMakeLists.txt`（CMake 配置示例）

### 核心 Modbus 文件
1. `modbus/include/mbport.h`（Modbus 端口接口定义）
2. `modbus/mb.c`（Modbus 主实现）

## 许可证说明
- 端口文件（`port/*.c`, `port/*.h`）：LGPL 许可证
- 演示应用（`demo.c`）：GPL 许可证
- LGPL 允许在特定条件下将端口文件集成到专有软件中
- GPL 要求任何衍生作品也使用 GPL 许可证

## 总结

STM32 移植的关键在于理解 ST 处理器 UART 配置的特殊性、正确配置 TIM7 定时器以及适当的中断优先级设置。此移植设计具有广泛的兼容性，支持多种 STM32 系列和 Nucleo 板卡。遵循上述步骤和注意事项，可快速将 FreeModbus 集成到 STM32 项目中。
### 1. 项目概述
这个项目是使用Rust语言移植的原来使用c++语言编写的OpenATC智能交通控制系统，项目名称 "trae-atc" (AI Traffic Control System)。

### 2. 项目架构
项目采用模块化设计，主要包含以下核心模块：

#### 2.1 核心模块
1. **控制模块** (<mcfolder name="control" path="/Users/xnpeng/RustroverProjects/trae-atc/src/control"></mcfolder>)
- 负责交通信号控制的核心逻辑
- 支持多种控制模式：
  - 定时控制 (Fixed)
  - 车辆响应 (Vehicle)
  - 闪光模式 (Flash)
  - 全红模式 (AllRed)
  - 手动模式 (Manual)
  - 错误状态 (Error)

2. **硬件模块** (<mcfolder name="hardware" path="/Users/xnpeng/RustroverProjects/trae-atc/src/hardware"></mcfolder>)
- 管理硬件设备
- 处理 CAN 总线通信
- 支持交通信号灯等设备控制

3. **通信模块** (<mcfolder name="communication" path="/Users/xnpeng/RustroverProjects/trae-atc/src/communication"></mcfolder>)
- 负责系统间通信

4. **故障处理模块** (<mcfolder name="fault" path="/Users/xnpeng/RustroverProjects/trae-atc/src/fault"></mcfolder>)
- 处理系统故障和异常情况

### 3. 技术特点

1. **异步编程**
- 使用 tokio 作为异步运行时
- 支持高并发操作

2. **硬件通信**
- 使用 socketcan 进行 CAN 总线通信
- 支持实时硬件控制

3. **数据处理**
- 使用 serde 进行序列化/反序列化
- 支持 JSON 和 XML 数据格式

4. **错误处理**
- 使用 thiserror 和 anyhow 进行错误处理
- 提供清晰的错误追踪

5. **日志系统**
- 使用 tracing 进行日志记录
- 支持结构化日志输出

### 4. 核心功能

1. **交通信号控制**
- 多种控制模式切换
- 配时方案管理
- 相位控制

2. **硬件管理**
- 设备配置
- 参数管理
- 实时监控

3. **系统监控**
- 故障检测
- 状态监控
- 日志记录

### 5. 项目特点

1. **模块化设计**
- 清晰的模块划分
- 高内聚低耦合

2. **可扩展性**
- 支持新控制模式添加
- 支持新硬件设备接入

3. **可靠性**
- 完善的错误处理
- 故障检测和恢复机制

4. **可维护性**
- 完整的测试用例
- 清晰的代码结构

这个项目展现了一个专业的交通控制系统架构，具有良好的可扩展性和可维护性，适合用于实际的交通控制场景。

# 初版进度

分析两个版本的交通信号控制系统：

## Rust 版本已实现的核心功能：

1. 硬件控制层
   
   - CAN通信
   - 相位控制
   - 参数管理
   - 设备监控
2. 控制逻辑层
   
   - 定周期控制
   - 车辆响应
   - 闪光控制
   - 全红控制
   - 手动控制
3. 系统功能
   
   - 故障管理
   - 数据存储
   - 系统监控
   - Web接口
   - 优化算法

## C++版本额外实现的功能：

1. 协调控制
   
   - 多路口协调
   - 绿波控制
   - 时间同步

2. 高级控制策略
   
   - 自适应控制
   - 优先控制
   - 特殊事件响应

3. 系统功能
   
   - 完整的配置管理
   - 数据统计分析
   - 远程管理接口
   - 系统诊断

# 后续计划：

1. 补充Rust版本缺失的功能模块
2. 接入强化学习智能体进行信号控制

# 第1次测试main.rs -- 2025-2-24 - tag20250224

```
2025-02-24T02:48:08.754386Z  INFO traeatc: 正在启动信号控制系统...
2025-02-24T02:48:08.899417Z  INFO traeatc::hardware::monitor: 硬件监控已启动: TSC001
2025-02-24T02:48:08.899443Z  INFO traeatc::hardware::monitor: 性能指标收集已启动: TSC001
2025-02-24T02:48:08.899516Z  INFO traeatc::control::manager: 初始化交通控制管理器
2025-02-24T02:48:08.899554Z  INFO traeatc::control::manager: 控制模式切换为: AllRed
```

## MockCanSocket

在communication模块中编写MockCanSocket模拟CAN接口，以便于在没有硬件连接的情况下测试系统功能；
在硬件配置代码里指明CAN接口为"mock*"，以后在建立CanConnection时，发现can_interface为"mock*"时，
就使用MockCanSocket模拟通信功能。

``` 
// 初始化硬件管理器
    let hardware_config = HardwareConfig {
        hardware_id: "TSC001".to_string(),
        hardware_type: HardwareType::Controller,
        can_interface: "mock0".to_string(),
        parameters: Default::default(),
    };
```

``` 
//模拟CAN接口
let socket: Box<dyn CanSocket + Send> = if interface.starts_with("mock") 
```

### system_test

```cargo test  -- --nocapture```

``` 
test hardware::tests::mock::tests::test_mock_can_connection ... ok
test hardware::tests::mock::tests::test_mock_hardware_manager ... ok
test hardware::tests::hardware_tests::test_error_handling ... FAILED
test hardware::tests::hardware_tests::test_hardware_initialization ... FAILED
test hardware::tests::hardware_tests::test_phase_management ... FAILED
test hardware::tests::hardware_tests::test_parameter_management ... FAILED
test hardware::tests::hardware_tests::test_monitor_functionality ... FAILED

failures:
    hardware::tests::hardware_tests::test_error_handling
    hardware::tests::hardware_tests::test_hardware_initialization
    hardware::tests::hardware_tests::test_monitor_functionality
    hardware::tests::hardware_tests::test_parameter_management
    hardware::tests::hardware_tests::test_phase_management

test result: FAILED. 5 passed; 5 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s
```

### 修改主程序循环

``` 
2025-02-27T01:10:45.083433Z  INFO traeatc: 正在启动信号控制系统...
2025-02-27T01:10:45.222963Z  INFO traeatc::hardware::monitor: 硬件监控已启动: TSC001
2025-02-27T01:10:45.222989Z  INFO traeatc::hardware::monitor: 性能指标收集已启动: TSC001
2025-02-27T01:10:45.223133Z  INFO traeatc::control::manager: 从配置文件加载相位和配时方案
2025-02-27T01:10:45.246629Z  INFO traeatc: 信号控制系统已启动
运行时间: 8秒 601963166纳秒2025-02-27T01:10:53.911018Z  WARN traeatc::monitor::manager: 系统资源使用率过高
运行时间: 9秒 401788875纳秒2025-02-27T01:10:54.706158Z  WARN traeatc::monitor::manager: 系统资源使用率过高
运行时间: 9秒 501422750纳秒2025-02-27T01:10:54.808836Z  WARN traeatc::monitor::manager: 系统资源使用率过高
运行时间: 9秒 601785791纳秒2025-02-27T01:10:54.911408Z  WARN traeatc::monitor::manager: 系统资源使用率过高
运行时间: 58秒 2076000纳秒2025-02-27T01:11:43.302578Z  WARN traeatc::monitor::manager: 系统资源使用率过高
运行时间: 165秒 902762666纳秒

```
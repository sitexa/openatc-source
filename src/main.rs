use std::io;
use std::io::Write;
use std::sync::Arc;
use std::time::Instant;
use tokio::time::Duration;
use tracing::{error, info, Level};
use tracing_subscriber::FmtSubscriber;

use control::{types::ControlMode, ControlManager};
use fault::{types::FaultConfig, FaultManager};
use hardware::{HardwareConfig, HardwareManager, HardwareType};
use monitor::MonitorManager;

pub mod hardware;
pub mod control;
pub mod communication;
pub mod fault;
pub mod storage;
pub mod monitor;
pub mod coordination;
pub mod interface;
pub mod optimization;

#[tokio::main]
async fn main() {
    // 初始化日志系统
    let subscriber = FmtSubscriber::builder()
        .with_max_level(Level::INFO)
        .init();

    info!("正在启动信号控制系统...");

    // 初始化硬件管理器
    let hardware_config = HardwareConfig {
        hardware_id: "TSC001".to_string(),
        hardware_type: HardwareType::Controller,
        can_interface: "mock0".to_string(), //使用MockCanSocket
        parameters: Default::default(),
    };

    let hardware = match HardwareManager::new(hardware_config).await {
        Ok(hw) => Arc::new(hw),
        Err(e) => {
            error!("硬件初始化失败: {}", e);
            return;
        }
    };

    // 初始化控制管理器
    let control = Arc::new(ControlManager::new(hardware.clone()));

    // 初始化监控和故障管理
    let monitor = Arc::new(MonitorManager::new());

    // 初始化故障管理器
    let fault = Arc::new(FaultManager::new(FaultConfig {
        max_events: 1000,
        auto_resolve_timeout: Some(Duration::from_secs(3600)),
        notification_enabled: true,
    }));

    // 启动各个组件
    if let Err(e) = hardware.initialize().await {
        error!("硬件启动失败: {}", e);
        return;
    }

    if let Err(e) = control.initialize().await {
        error!("控制器启动失败: {}", e);
        return;
    }

    // 设置初始控制模式：定周期
    if let Err(e) = control.set_mode(ControlMode::Fixed).await {
        error!("控制模式设置失败: {}", e);
        return;
    }

    info!("信号控制系统已启动");

    let start_time = Instant::now(); // 记录程序开始时间
    let mut interval = tokio::time::interval(Duration::from_millis(100)); // 设置100ms的执行周期

    // 主循环
    loop {
        tokio::select! {
            // 等待定时器触发
            _ = interval.tick() => {
                // Clone Arc references for each task
                let control = control.clone();
                let monitor = monitor.clone();
                let fault = fault.clone();

                // 异步启动所有任务
                tokio::spawn(async move{
                    if let Err(e) = control.run_cycle().await {
                        error!("控制循环执行失败: {:?}", e);
                    }
                });
                
                tokio::spawn(async move{
                    if let Err(e) = monitor.update().await {
                        error!("监控更新失败: {:?}", e);
                    }
                });
                
                tokio::spawn(async move{
                    if let Err(e) = fault.monitor_faults().await {
                        error!("故障检测失败: {:?}", e);
                    }
                });

                // 计算并显示运行时间
                let elapsed = start_time.elapsed();
                print!("\r运行时间: {}秒 {}纳秒", elapsed.as_secs(), elapsed.subsec_nanos());
                io::stdout().flush().unwrap();
            }
            
            // 系统退出处理
            _ = tokio::signal::ctrl_c() => {
                info!("\r接收到退出信号，正在关闭系统...");
                break;
            }
        }
    }

    // 优雅关闭
    if let Err(e) = control.set_mode(ControlMode::AllRed).await {
        error!("关闭时设置全红失败: {}", e);
    }
    hardware.shutdown().await.expect("系统关闭失败");
    info!("系统已安全关闭");
}
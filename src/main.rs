use std::sync::Arc;
use tokio::time::Duration;
use tracing::{info, error, Level};
use tracing_subscriber::FmtSubscriber;

use control::{ControlManager, types::ControlMode};
use fault::{FaultManager, types::FaultConfig};
use monitor::MonitorManager;
use hardware::{HardwareManager, HardwareConfig, HardwareType};

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
        can_interface: "mock0".to_string(),
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
        error!("控制器初始化失败: {}", e);
        return;
    }

    // 设置初始控制模式
    if let Err(e) = control.set_mode(ControlMode::Fixed).await {
        error!("控制模式设置失败: {}", e);
        return;
    }

    info!("信号控制系统已启动");

    // 主循环
    loop {
        tokio::select! {
            // 处理控制循环
            _ = control.run_cycle() => {},
            
            // 处理监控更新
            _ = monitor.update() => {},
            
            // 处理故障检测
            _ = fault.monitor_faults() => {},
            
            // 系统退出处理
            _ = tokio::signal::ctrl_c() => {
                info!("接收到退出信号，正在关闭系统...");
                break;
            }
        }
    }

    // 优雅关闭
    if let Err(e) = control.set_mode(ControlMode::AllRed).await {
        error!("关闭时设置全红失败: {}", e);
    }
    hardware.shutdown().await;
    info!("系统已安全关闭");
}
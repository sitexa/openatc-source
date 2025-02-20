use std::sync::Arc;
use tokio::time::{Duration, interval};
use tracing::{info, error, Level};
use tracing_subscriber::FmtSubscriber;

use hardware::{HardwareManager, types::{HardwareConfig, DeviceType}};
use control::ControlManager;
use fault::{FaultManager, types::{FaultConfig, FaultType, FaultSeverity}};
use storage::{StorageManager, types::StorageConfig};
use monitor::MonitorManager;
use interface::web::WebServer;

pub mod hardware;
pub mod control;
pub mod communication;
pub mod fault;
// 删除 pub mod config;
pub mod storage;
pub mod monitor;
pub mod coordination;
pub mod interface;
pub mod optimization;

#[tokio::main]
async fn main() {
    // 初始化日志系统
    let subscriber = FmtSubscriber::builder()
        .with_max_level(Level::DEBUG)
        .init();

    info!("OpenATC 系统启动...");

    let fault = Arc::new(FaultManager::new(FaultConfig {
        max_events: 1000,
        auto_resolve_timeout: Some(Duration::from_secs(3600)),
        notification_enabled: true,
    }));

    let hardware = match HardwareManager::new(hardware::types::HardwareConfig {
        device_id: "".to_string(),
        device_type: DeviceType::TrafficLight,
        can_interface: "can0".to_string(),
        parameters: Default::default(),
    }).await {
        Ok(hw) => Arc::new(hw),
        Err(e) => {
            error!("硬件管理器初始化失败: {}", e);
            return;
        }
    };

    let control = Arc::new(ControlManager::new(hardware.clone()));
    if let Err(e) = control.initialize().await {
        error!("控制管理器初始化失败: {}", e);
        return;
    }

    let storage = match StorageManager::new(StorageConfig {
        data_dir: "data".to_string(),
        max_file_size: 1024 * 1024,  // 1MB
        max_history_days: 30,
        auto_cleanup: true,
    }) {
        Ok(sm) => Arc::new(sm),
        Err(e) => {
            error!("存储管理器初始化失败: {}", e);
            return;
        }
    };

    let monitor = Arc::new(MonitorManager::new());

    // 启动Web服务器
    let web_server = WebServer::new(control.clone(), 8080);
    tokio::spawn(async move {
        web_server.start().await;
    });

    // 主循环
    let mut interval = interval(Duration::from_millis(100));
    loop {
        interval.tick().await;

        // 运行控制周期
        if let Err(e) = control.run_cycle().await {
            error!("控制周期执行失败: {}", e);
            fault.report_fault(
                fault::types::FaultType::System,
                fault::types::FaultSeverity::Major,
                format!("控制周期错误: {}", e),
            ).await.ok();
        }

        // 更新系统监控
        if let Err(e) = monitor.update().await {
            error!("系统监控更新失败: {}", e);
        }

        // 存储运行数据
        if let Err(e) = storage.store_signal_data(control.get_status().await.into()).await {
            error!("数据存储失败: {}", e);
        }
    }

    info!("OpenATC 系统退出");
}
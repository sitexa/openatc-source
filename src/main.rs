use std::sync::Arc;
use std::time::SystemTime;
use chrono::format::Numeric::Timestamp;
use tokio::time::Duration;
use tracing::{info, error, Level};
use tracing_subscriber::FmtSubscriber;

use control::{ControlManager, types::{ControlMode, PhaseState}};
use fault::{FaultManager, types::{FaultConfig, FaultType, FaultSeverity}};
use storage::{StorageManager, types::StorageConfig};
use monitor::MonitorManager;
use crate::storage::{DetectorData, PhaseStateData, SignalData};
use hardware::{
    HardwareManager, 
    HardwareConfig, 
    HardwareType,
    HardwareValue,
};

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
        .with_max_level(Level::DEBUG)
        .init();

    info!("开始功能测试...");

    // 3. 测试故障管理
    info!("测试故障管理...");
    let fault = Arc::new(FaultManager::new(FaultConfig {
        max_events: 1000,
        auto_resolve_timeout: Some(Duration::from_secs(3600)),
        notification_enabled: true,
    }));

    // 报告测试故障
    let fault_id = fault.report_fault(
        FaultType::System,
        FaultSeverity::Minor,
        "测试故障".to_string(),
    ).await.expect("故障报告失败");

    // 解决故障
    fault.resolve_fault(fault_id).await.expect("故障解决失败");

    // 4. 测试数据存储
    info!("测试数据存储...");
    let storage = Arc::new(StorageManager::new(StorageConfig {
        data_dir: "test_data".to_string(),
        max_file_size: 1024 * 1024,
        max_history_days: 7,
        auto_cleanup: true,
    }).expect("存储初始化失败"));

    // 存储测试数据
    let data = SignalData {
        timestamp: SystemTime::now(),
        signal_id: 1,
        phase_states: vec![
            PhaseStateData {
                phase_id: 1,
                state: "Green".to_string(),
                elapsed_time: 15,
                remaining_time: 25,
            },
            PhaseStateData {
                phase_id: 2,
                state: "Red".to_string(),
                elapsed_time: 40,
                remaining_time: 0,
            },
        ],
        detector_states: vec![
            DetectorData {
                detector_id: 1,
                occupancy: 0.35,
                volume: 12,
                speed: Some(45.5),
            },
            DetectorData {
                detector_id: 2,
                occupancy: 0.28,
                volume: 8,
                speed: Some(38.2),
            },
        ],
    };

    storage.store_signal_data(data)
        .await.expect("数据存储失败");

    // 5. 测试系统监控
    info!("测试系统监控...");
    let monitor = Arc::new(MonitorManager::new());
    monitor.update().await.expect("监控更新失败");

    // 等待一段时间观察系统运行状态
    tokio::time::sleep(Duration::from_secs(5)).await;

    info!("功能测试完成");
}
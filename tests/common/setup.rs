use std::sync::Arc;
use tokio::sync::mpsc;
use traeatc::hardware::{HardwareManager, HardwareConfig, HardwareType};
use traeatc::communication::types::CanMessage;

pub type MockMessage = CanMessage;

pub async fn setup_mock_hardware() -> Arc<HardwareManager> {
    let config = HardwareConfig {
        hardware_id: "TSC001".to_string(),
        hardware_type: HardwareType::Controller,
        can_interface: "mock0".to_string(),
        parameters: Default::default(),
    };

    Arc::new(HardwareManager::new(config).await.expect("Failed to create mock hardware"))
}

pub async fn collect_mock_messages() -> Vec<MockMessage> {
    // 等待一段时间收集消息
    tokio::time::sleep(tokio::time::Duration::from_millis(100)).await;

    // TODO: 实现从 mock 设备收集消息的逻辑
    Vec::new()  // 临时返回空集合
}
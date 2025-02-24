use std::sync::Arc;
use traeatc::control::{ControlManager, types::ControlMode};
mod common;
use common::setup::{setup_mock_hardware, collect_mock_messages};


#[tokio::test]
async fn test_signal_control() {
    // 初始化系统组件
    let hardware = setup_mock_hardware().await;
    let control = Arc::new(ControlManager::new(hardware.clone()));
    
    // 测试相位切换
    control.set_mode(ControlMode::Fixed).await.unwrap();
    
    // 验证输出信号
    let messages = collect_mock_messages().await;
    assert_eq!(messages.len(), 8); // 应该有8个相位的信号
    
    // 验证相位状态
    for msg in messages {
        assert!(msg.id >= 0x300 && msg.id < 0x308);
    }
}

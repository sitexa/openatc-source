use traeatc::control::{ControlManager, TimingPlan, Phase, ControlMode};
use traeatc::hardware::HardwareManager;
use std::sync::Arc;

#[tokio::test]
async fn test_control_mode_change() {
    // 创建测试用硬件管理器
    let hardware = Arc::new(HardwareManager::new(/* config */).await.unwrap());
    let control = ControlManager::new(hardware);

    // 测试模式切换
    assert!(control.set_mode(ControlMode::AllRed).await.is_ok());
    assert!(control.set_mode(ControlMode::Fixed).await.is_ok());
}
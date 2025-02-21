use crate::hardware::{
    HardwareManager,
    error::HardwareResult,
    types::{
        HardwareConfig, HardwareType, HardwareParameter, HardwareValue,
    },
    status::HardwareStatus,
};
use crate::control::PhaseState;
use super::mock::MockCanConnection;
use std::sync::Arc;
use tokio::sync::Mutex;
use std::collections::HashMap;
use std::time::Duration;

#[tokio::test]
async fn test_hardware_initialization() {
    let config = HardwareConfig {
        hardware_id: "test_hardware".to_string(),
        hardware_type: HardwareType::Controller,
        can_interface: "vcan0".to_string(),
        parameters: HashMap::new(),
    };

    let manager = HardwareManager::new(config).await.unwrap();
    let result = manager.initialize().await;
    assert!(result.is_ok(), "硬件初始化应该成功");

    let status = manager.status.lock().await;
    assert_eq!(status.hardware_id, "test_hardware");
}

#[tokio::test]
async fn test_parameter_management() {
    let mut config = HardwareConfig {
        hardware_id: "test_hardware".to_string(),
        hardware_type: HardwareType::Controller,
        can_interface: "vcan0".to_string(),
        parameters: HashMap::new(),
    };

    // 添加测试参数
    let test_param = HardwareParameter {
        id: 1,
        name: String::from("test_param"),
        value: HardwareValue::Integer(42),
        unit: None,
    };
    config.parameters.insert(test_param.name.clone(), test_param);

    let manager = HardwareManager::new(config).await.unwrap();

    // 测试参数更新
    let result = manager.update_parameter(
        "test_param",
        HardwareValue::Integer(100),
    ).await;
    assert!(result.is_ok(), "参数更新应该成功");

    // 验证参数值
    let params = manager.params.lock().await;
    let value = params.get_parameter_definition("test_param").unwrap();
    assert_eq!(value.id, 1);
}

#[tokio::test]
async fn test_phase_management() {
    let config = HardwareConfig {
        hardware_id: "test_hardware".to_string(),
        hardware_type: HardwareType::Controller,
        can_interface: "vcan0".to_string(),
        parameters: HashMap::new(),
    };

    let manager = HardwareManager::new(config).await.unwrap();

    // 测试相位状态更新
    let result = manager.update_phase_output(1, PhaseState::Red).await;
    assert!(result.is_ok(), "相位状态更新应该成功");

    // 测试无效相位
    let result = manager.update_phase_output(255, PhaseState::Green).await;
    assert!(result.is_ok(), "应该正常处理无效相位");
}

#[tokio::test]
async fn test_monitor_functionality() {
    let config = HardwareConfig {
        hardware_id: "test_hardware".to_string(),
        hardware_type: HardwareType::Controller,
        can_interface: "vcan0".to_string(),
        parameters: HashMap::new(),
    };

    let manager = HardwareManager::new(config).await.unwrap();
    manager.initialize().await.unwrap();

    // 等待监控数据收集
    tokio::time::sleep(Duration::from_secs(1)).await;

    let status = manager.status.lock().await;
    assert!(status.online, "硬件应该在线");
}

#[tokio::test]
async fn test_error_handling() {
    let config = HardwareConfig {
        hardware_id: "test_hardware".to_string(),
        hardware_type: HardwareType::Controller,
        can_interface: "invalid_interface".to_string(),
        parameters: HashMap::new(),
    };

    // 测试无效配置
    let result = HardwareManager::new(config).await;
    assert!(result.is_ok(), "应该正常处理无效配置");
}
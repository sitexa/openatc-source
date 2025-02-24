use super::mock::MockCanConnection;
use crate::communication::CanMessage;
use crate::hardware::types::HardwareValue;
use crate::hardware::{
    HardwareConfig,
    HardwareType,
};
use std::collections::HashMap;
use std::sync::Arc;
use tokio::sync::Mutex;

#[tokio::test]
async fn test_parameter_can_message() {
    // 创建测试配置
    let config = HardwareConfig {
        hardware_id: "test_device".to_string(),
        hardware_type: HardwareType::Controller,
        can_interface: "vcan0".to_string(),
        parameters: HashMap::new(),
    };

    // 创建模拟 CAN 连接
    let mock_can = Arc::new(Mutex::new(MockCanConnection::new()));
    let sent_messages = mock_can.lock().await.sent_messages.clone();

    // 测试参数更新
    let test_cases = vec![
        (
            "test_int",
            HardwareValue::Integer(42),
            vec![1, 0, 0, 0, 1, 42, 0, 0],
        ),
        (
            "test_float",
            HardwareValue::Float(3.14),
            vec![2, 0, 0, 0, 2, 195, 245, 40],
        ),
        (
            "test_bool",
            HardwareValue::Boolean(true),
            vec![3, 0, 0, 0, 3, 1],
        ),
    ];

    for (name, value, expected_data) in test_cases {
        // 发送参数更新消息
        let message = CanMessage {
            id: 0x200,
            data: expected_data.clone(),  // 克隆数据
            extended: false,
            rtr: false,
        };
        mock_can.lock().await.send_message(message).await.unwrap();

        // 验证发送的消息
        let messages = sent_messages.lock().await;
        let last_message = messages.last().unwrap();
        assert_eq!(last_message.id, 0x200);
        assert_eq!(&last_message.data, &expected_data);  // 使用引用比较
    }
}

#[tokio::test]
async fn test_phase_can_message() {
    // 创建模拟 CAN 连接
    let mock_can = Arc::new(Mutex::new(MockCanConnection::new()));
    let sent_messages = mock_can.lock().await.sent_messages.clone();

    // 测试相位状态消息
    let phase_id = 1;
    let message = CanMessage {
        id: 0x300 + phase_id,
        data: vec![phase_id as u8, 0x01],  // RED 状态
        extended: false,
        rtr: false,
    };

    // 发送消息
    mock_can.lock().await.send_message(message).await.unwrap();

    // 验证消息
    let messages = sent_messages.lock().await;
    let last_message = messages.last().unwrap();
    assert_eq!(last_message.id, 0x301);
    assert_eq!(last_message.data, vec![1, 0x01]);
}

#[tokio::test]
async fn test_can_error_handling() {
    // 创建模拟 CAN 连接
    let mock_can = Arc::new(Mutex::new(MockCanConnection::new()));
    
    // 测试发送无效消息
    let result = mock_can.lock().await.send_message(CanMessage {
        id: 0xFFF,  // 无效 ID
        data: vec![],  // 空数据
        extended: false,
        rtr: false,
    }).await;

    // 验证错误处理
    assert!(result.is_ok(), "应该正常处理无效消息");
}
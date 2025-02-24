use crate::communication::CanMessage;
use crate::hardware::types::HardwareValue;
use crate::hardware::{
    HardwareConfig, HardwareParameter, HardwareResult
    , HardwareStatus,
};
use std::collections::HashMap;
use std::sync::Arc;
use tokio::sync::Mutex;

pub struct MockCanConnection {
    pub sent_messages: Arc<Mutex<Vec<CanMessage>>>,
}

impl MockCanConnection {
    pub fn new() -> Self {
        Self {
            sent_messages: Arc::new(Mutex::new(Vec::new())),
        }
    }

    pub async fn send_message(&mut self, message: CanMessage) -> HardwareResult<()> {
        self.sent_messages.lock().await.push(message);
        Ok(())
    }
}

pub struct MockHardwareManager {
    pub config: HardwareConfig,
    pub types: HardwareStatus,
    pub parameters: HashMap<String, HardwareParameter>,
    pub sent_messages: Arc<Mutex<Vec<CanMessage>>>,
}

impl MockHardwareManager {
    pub fn new() -> Self {
        Self {
            config: HardwareConfig {
                hardware_id: "mock_hardware".to_string(),
                hardware_type: crate::hardware::HardwareType::Controller,
                can_interface: "mock_can".to_string(),
                parameters: HashMap::new(),
            },
            types: HardwareStatus::new("mock_hardware".to_string()),
            parameters: HashMap::new(),
            sent_messages: Arc::new(Mutex::new(Vec::new())),
        }
    }

    pub fn add_parameter(&mut self, name: &str, value: HardwareValue) {
        self.parameters.insert(name.to_string(), HardwareParameter {
            id: self.parameters.len() as u32,
            name: name.to_string(),
            value,
            unit: None,
        });
    }

    pub async fn get_last_message(&self) -> Option<CanMessage> {
        let messages = self.sent_messages.lock().await;
        messages.last().cloned()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use tokio::test;

    #[test]
    async fn test_mock_hardware_manager() {
        let mut mock = MockHardwareManager::new();
        
        // 测试参数添加
        mock.add_parameter("test_param", HardwareValue::Integer(42));
        assert!(mock.parameters.contains_key("test_param"));
        
        // 测试消息发送
        let message = CanMessage {
            id: 0x123,
            data: vec![1, 2, 3],
            extended: false,
            rtr: false,
        };
        mock.sent_messages.lock().await.push(message.clone());
        
        let last_message = mock.get_last_message().await.unwrap();
        assert_eq!(last_message.id, 0x123);
    }

    #[test]
    async fn test_mock_can_connection() {
        let mut mock_can = MockCanConnection::new();
        
        let message = CanMessage {
            id: 0x456,
            data: vec![4, 5, 6],
            extended: false,
            rtr: false,
        };
        
        mock_can.send_message(message.clone()).await.unwrap();
        
        let sent_messages = mock_can.sent_messages.lock().await;
        assert_eq!(sent_messages.len(), 1);
        assert_eq!(sent_messages[0].id, 0x456);
    }
}
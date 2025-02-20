use traeatc::hardware::{HardwareConfig, DeviceType, Parameter, ParameterValue, HardwareManager};
use std::collections::HashMap;

#[tokio::test]
async fn test_hardware_manager_creation() {
    let mut parameters = HashMap::new();
    parameters.insert(
        "signal_timing".to_string(),
        Parameter {
            id: 1,
            name: "signal_timing".to_string(),
            value: ParameterValue::Integer(30),
            unit: Some("seconds".to_string()),
        },
    );

    let config = HardwareConfig {
        device_id: "TEST001".to_string(),
        device_type: DeviceType::TrafficLight,
        can_interface: "can0".to_string(),
        parameters,
    };

    let manager = HardwareManager::new(config).await;
    assert!(manager.is_ok());
}
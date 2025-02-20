use super::{error::{HardwareError, HardwareResult}, types::*};
use crate::communication::can::CanConnection;
use tokio::sync::Mutex;
use std::sync::Arc;
use tracing::{info, error};
use crate::communication::CanMessage;
use crate::control::PhaseState;
use crate::hardware::monitor::DeviceMonitor;
use crate::hardware::params::ParameterStore;

pub struct HardwareManager {
    config: Arc<Mutex<HardwareConfig>>,
    can_connection: Arc<Mutex<CanConnection>>,
    status: Arc<Mutex<DeviceStatus>>,
    monitor: Arc<DeviceMonitor>,
    params: Arc<Mutex<ParameterStore>>,
}

#[derive(Debug, Clone)]
pub struct DeviceStatus {
    pub is_connected: bool,
    pub last_heartbeat: std::time::SystemTime,
    pub error_count: u32,
}

impl HardwareManager {
    pub async fn new(config: HardwareConfig) -> HardwareResult<Self> {
        let monitor = DeviceMonitor::new(config.device_id.clone());
        let params = Arc::new(Mutex::new(ParameterStore::new()));

        let can_connection = CanConnection::new(&config.can_interface)
            .await
            .map_err(|e| HardwareError::InitializationError(e.to_string()))?;

        let status = DeviceStatus {
            is_connected: false,
            last_heartbeat: std::time::SystemTime::now(),
            error_count: 0,
        };

        Ok(Self {
            config: Arc::new(Mutex::new(config)),
            can_connection: Arc::new(Mutex::new(can_connection)),
            status: Arc::new(Mutex::new(status)),
            monitor: Arc::new(monitor),
            params,
        })
    }
    pub async fn initialize(&self) -> HardwareResult<()> {
        // 启动设备监控
        self.monitor.start_monitoring().await?;
        // Start metrics collection
        self.monitor.start_metrics_collection().await?;

        // 加载参数配置
        if let Ok(store) = ParameterStore::load_from_file("params.json") {
            let mut params = self.params.lock().await;
            *params = store;
        }
        Ok(())
    }
    pub async fn update_parameter(&self, name: &str, value: ParameterValue) -> HardwareResult<()> {
        let mut params = self.params.lock().await;
        params.validate_parameter(name, &value)?;
    
        // 先获取所需信息
        let param_id = match params.get_parameter_definition(name) {
            Some(def) => def.id,
            None => return Err(HardwareError::ParameterError(format!("参数未定义: {}", name))),
        };
    
        // 创建 CAN 消息
        let can_msg = CanMessage {
            id: 0x200 + param_id,
            data: self.serialize_parameter_value(&param_id, value.clone()),
            extended: false,
            rtr: false,
        };
    
        // 更新参数值
        params.set_value(name, value.clone())?;
    
        // 发送 CAN 消息
        self.can_connection.lock().await
            .send_message(can_msg)
            .await
            .map_err(|e| HardwareError::CommunicationError(e.to_string()))?;
    
        // 保存配置
        params.save_to_file("params.json")?;
        info!("参数已更新: {} = {:?}", name, value);
        Ok(())
    }
    async fn create_parameter_message(&self, param: &Parameter) -> HardwareResult<CanMessage> {
        // Create a simple parameter update message
        let mut data = Vec::with_capacity(6);
        data.extend_from_slice(&param.id.to_le_bytes());

        match &param.value {
            ParameterValue::Integer(v) => {
                data.push(1); // type identifier for integer
                data.extend_from_slice(&(*v as i32).to_le_bytes());
            }
            ParameterValue::Float(v) => {
                data.push(2); // type identifier for float
                data.extend_from_slice(&(*v as f32).to_le_bytes());
            }
            ParameterValue::Boolean(v) => {
                data.push(3); // type identifier for boolean
                data.push(if *v { 1 } else { 0 });
            }
            ParameterValue::String(v) => {
                data.push(4); // type identifier for string
                data.extend_from_slice(v.as_bytes());
            }
        }

        Ok(CanMessage {
            id: 0x200 + param.id, // Base ID + parameter ID
            data,
            extended: false,
            rtr: false,
        })
    }

    fn serialize_parameter_value(&self, param_id: &u32, value: ParameterValue) -> Vec<u8> {
        let mut data = Vec::with_capacity(8);
        data.extend_from_slice(&param_id.to_le_bytes());

        match value {
            ParameterValue::Integer(v) => {
                data.push(1); // 整数类型标识
                data.extend_from_slice(&v.to_le_bytes()[..4]); // 取前4字节
            }
            ParameterValue::Float(v) => {
                data.push(2); // 浮点数类型标识
                data.extend_from_slice(&v.to_le_bytes());
            }
            ParameterValue::Boolean(v) => {
                data.push(3); // 布尔类型标识
                data.push(if v { 1 } else { 0 });
            }
            ParameterValue::String(v) => {
                data.push(4); // 字符串类型标识
                data.extend(v.as_bytes().iter().take(7)); // 最多取7个字节
            }
        }

        data
    }

    pub async fn update_phase_output(&self, phase_id: u32, state: PhaseState) -> HardwareResult<()> {
        // 将相位状态转换为CAN消息
        let message = self.create_phase_output_message(phase_id, state)?;
        
        // 通过CAN总线发送消息
        self.can_connection.lock().await
            .send_message(message)
            .await
            .map_err(|e| HardwareError::CommunicationError(e.to_string()))?;

        info!("相位输出更新: 相位 {} -> {:?}", phase_id, state);
        Ok(())
    }

    fn create_phase_output_message(&self, phase_id: u32, state: PhaseState) -> HardwareResult<CanMessage> {
        let mut data = Vec::with_capacity(2);
        data.push(phase_id as u8);
        
        let state_byte = match state {
            PhaseState::Red => 0x01,
            PhaseState::Yellow => 0x02,
            PhaseState::Green => 0x03,
            PhaseState::Flash => 0x04,
        };
        data.push(state_byte);

        Ok(CanMessage {
            id: 0x300 + phase_id, // 基础ID + 相位ID
            data,
            extended: false,
            rtr: false,
        })
    }
}
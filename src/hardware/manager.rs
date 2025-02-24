use super::{error::{HardwareError, HardwareResult}, types::HardwareConfig};
use crate::hardware::params::{HardwareParameter};  // 使用完整路径
use crate::communication::can::CanConnection;
use crate::hardware::types::HardwareValue;
use tokio::sync::Mutex;
use std::sync::Arc;
use tracing::{info, error};
use crate::communication::CanMessage;
use crate::control::PhaseState;
use crate::hardware::monitor::HardwareMonitor;
use crate::hardware::status::HardwareStatus;  // 明确导入 HardwareStatus
use crate::hardware::params::ParameterStore;
pub struct HardwareManager {
    pub(crate) config: Arc<Mutex<HardwareConfig>>,
    pub(crate) can_connection: Arc<Mutex<CanConnection>>,
    pub(crate) status: Arc<Mutex<HardwareStatus>>,  // 改名
    pub(crate) monitor: Arc<HardwareMonitor>,  // 改名
    pub(crate) params: Arc<Mutex<ParameterStore>>,
}

impl HardwareManager {
    pub async fn new(config: HardwareConfig) -> HardwareResult<Self> {
        let monitor = HardwareMonitor::new(config.hardware_id.clone());
        let params = Arc::new(Mutex::new(ParameterStore::new()));

        let can_connection = CanConnection::new(&config.can_interface)
            .await
            .map_err(|e| HardwareError::InitializationError(e.to_string()))?;

        let status = HardwareStatus::new(config.hardware_id.clone());

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
    pub async fn update_parameter(&self, name: &str, value: HardwareValue) -> HardwareResult<()> {
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
    async fn create_parameter_message(&self, param: &HardwareParameter) -> HardwareResult<CanMessage> {
        let mut data = Vec::with_capacity(6);
        data.extend_from_slice(&param.id.to_le_bytes());

        match &param.value {
            HardwareValue::Integer(v) => {
                data.push(1);
                data.extend_from_slice(&(*v as i32).to_le_bytes());
            }
            HardwareValue::Float(v) => {
                data.push(2);
                data.extend_from_slice(&(*v as f32).to_le_bytes());
            }
            HardwareValue::Boolean(v) => {
                data.push(3);
                data.push(if *v { 1 } else { 0 });
            }
            HardwareValue::String(v) => {
                data.push(4);
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

    fn serialize_parameter_value(&self, param_id: &u32, value: HardwareValue) -> Vec<u8> {
        let mut data = Vec::with_capacity(8);
        data.extend_from_slice(&param_id.to_le_bytes());

        match value {
            HardwareValue::Integer(v) => {
                data.push(1);
                data.extend_from_slice(&v.to_le_bytes()[..4]);
            }
            HardwareValue::Float(v) => {
                data.push(2);
                data.extend_from_slice(&v.to_le_bytes());
            }
            HardwareValue::Boolean(v) => {
                data.push(3);
                data.push(if v { 1 } else { 0 });
            }
            HardwareValue::String(v) => {
                data.push(4);
                data.extend(v.as_bytes().iter().take(7));
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

    pub async fn shutdown(&self) -> HardwareResult<()> {
        info!("正在关闭硬件管理器...");
        
        // 保存参数状态
        let params = self.params.lock().await;
        params.save_to_file("params.json")?;
        
        // 更新状态
        let mut status = self.status.lock().await;
        status.online = false;
        
        info!("硬件管理器已关闭");
        Ok(())
    }
}
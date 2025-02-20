use super::types::*;
use crate::control::ControlManager;
use std::sync::Arc;
use std::time::SystemTime;

pub struct ApiService {
    control: Arc<ControlManager>,
}

impl ApiService {
    pub fn new(control: Arc<ControlManager>) -> Self {
        Self { control }
    }

    pub async fn get_intersection_status(&self, id: u32) -> ApiResponse<SignalStatus> {
        // 获取路口状态
        ApiResponse {
            success: true,
            message: "获取状态成功".to_string(),
            data: Some(SignalStatus {
                intersection_id: id,
                current_phase: self.control.get_current_phase().await,
                phase_elapsed_time: self.control.get_phase_elapsed_time().await,
                mode: self.control.get_current_mode().await.to_string(),
                coordination_active: self.control.is_coordinated().await,
            }),
            timestamp: SystemTime::now(),
        }
    }

    pub async fn execute_command(&self, command: RemoteCommand) -> ApiResponse<()> {
        // 执行远程命令
        ApiResponse {
            success: true,
            message: "命令执行成功".to_string(),
            data: None,
            timestamp: SystemTime::now(),
        }
    }
}
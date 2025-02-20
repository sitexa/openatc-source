use serde::{Deserialize, Serialize};
use std::time::SystemTime;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ApiResponse<T> {
    pub success: bool,
    pub message: String,
    pub data: Option<T>,
    pub timestamp: SystemTime,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SignalStatus {
    pub intersection_id: u32,
    pub current_phase: u32,
    pub phase_elapsed_time: u32,
    pub mode: String,
    pub coordination_active: bool,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct RemoteCommand {
    pub command_type: CommandType,
    pub intersection_id: u32,
    pub parameters: serde_json::Value,
}

#[derive(Debug, Clone, Copy, Serialize, Deserialize)]
pub enum CommandType {
    ChangeMode,
    UpdatePlan,
    ForcePhase,
    EnableCoordination,
    DisableCoordination,
    Heartbeat,
}
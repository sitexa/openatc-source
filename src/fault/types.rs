use serde::{Deserialize, Serialize};
use std::time::SystemTime;

#[derive(Debug, Clone, Copy, PartialEq, Serialize, Deserialize)]
pub enum FaultSeverity {
    Critical,    // 严重故障
    Major,       // 主要故障
    Minor,       // 次要故障
    Warning,     // 警告
}

#[derive(Debug, Clone, Copy, PartialEq, Serialize, Deserialize)]
pub enum FaultType {
    Hardware,    // 硬件故障
    Communication, // 通信故障
    Configuration, // 配置错误
    Timing,      // 配时错误
    Detector,    // 检测器故障
    System,      // 系统故障
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FaultEvent {
    pub id: u32,
    pub fault_type: FaultType,
    pub severity: FaultSeverity,
    pub description: String,
    pub timestamp: SystemTime,
    pub resolved: bool,
    pub resolution_time: Option<SystemTime>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FaultConfig {
    pub max_events: usize,
    pub auto_resolve_timeout: Option<std::time::Duration>,
    pub notification_enabled: bool,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FaultRecord {
    pub id: u32,
    pub fault_type: FaultType,
    pub severity: FaultSeverity,
    pub timestamp: SystemTime,
    pub description: String,
    pub source: String,
    pub status: FaultStatus,
    pub recovery_action: Option<String>,
}

#[derive(Debug, Clone, Copy, Serialize, Deserialize, PartialEq)]
pub enum FaultStatus {
    Active,
    Resolved,
    Acknowledged,
    Investigating,
}
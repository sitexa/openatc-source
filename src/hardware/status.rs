use std::time::{SystemTime, Duration};
use serde::{Serialize, Deserialize};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct HardwareStatus {
    pub hardware_id: String,
    pub last_heartbeat: SystemTime,
    pub online: bool,
    pub error_count: u32,
    pub can_status: bool,
    pub metrics: HardwareMetrics,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct HardwareMetrics {
    pub cpu_usage: f32,
    pub memory_usage: f32,
    pub temperature: f32,
    pub uptime: Duration,
    pub last_update: SystemTime,
}

impl HardwareStatus {
    pub fn new(hardware_id: String) -> Self {
        Self {
            hardware_id,
            last_heartbeat: SystemTime::now(),
            online: false,
            error_count: 0,
            can_status: false,
            metrics: HardwareMetrics::default(),
        }
    }

    pub fn update_heartbeat(&mut self) {
        self.last_heartbeat = SystemTime::now();
        self.online = true;
    }
}

impl Default for HardwareMetrics {
    fn default() -> Self {
        Self {
            cpu_usage: 0.0,
            memory_usage: 0.0,
            temperature: 0.0,
            uptime: Duration::default(),
            last_update: SystemTime::now(),
        }
    }
}
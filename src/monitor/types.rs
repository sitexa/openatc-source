use serde::{Deserialize, Serialize};
use std::time::{SystemTime, Duration};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SystemStatus {
    pub timestamp: SystemTime,
    pub cpu_usage: f32,
    pub memory_usage: f32,
    pub uptime: Duration,
    pub process_status: ProcessStatus,
    pub network_status: NetworkStatus,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ProcessStatus {
    pub thread_count: usize,
    pub open_files: usize,
    pub last_gc_time: SystemTime,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct NetworkStatus {
    pub can_connected: bool,
    pub message_rate: f32,
    pub error_count: u32,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PerformanceMetrics {
    pub cycle_time: Duration,
    pub response_time: Duration,
    pub queue_length: usize,
}

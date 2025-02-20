use serde::{Deserialize, Serialize};
use std::time::Duration;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TrafficState {
    pub volume: f32,          // 流量（辆/小时）
    pub occupancy: f32,       // 占有率（0-1）
    pub queue_length: f32,    // 排队长度（米）
    pub average_speed: f32,   // 平均速度（米/秒）
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PhaseOptimization {
    pub phase_id: u32,
    pub split_ratio: f32,     // 相位分配比例
    pub min_green: Duration,
    pub max_green: Duration,
    pub target_green: Duration,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct CycleOptimization {
    pub cycle_length: Duration,
    pub phases: Vec<PhaseOptimization>,
    pub performance_index: f32,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct QueueEstimate {
    pub approach_id: u32,
    pub queue_length: f32,    // 米
    pub vehicle_count: u32,
    pub delay: Duration,
    pub timestamp: std::time::SystemTime,
}
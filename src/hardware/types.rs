use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::time::{SystemTime, Duration};

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct HardwareConfig {
    pub hardware_id: String,      // 改为 hardware_id
    pub hardware_type: HardwareType,  // 改为 hardware_type
    pub can_interface: String,
    pub parameters: HashMap<String, HardwareParameter>,  // 改为 HardwareParameter
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub enum HardwareType {
    TrafficLight,
    Detector,
    Controller,
    Other(String),
}

#[derive(Debug, Clone)]
pub struct HardwareStatus {
    pub hardware_id: String,
    pub last_heartbeat: SystemTime,
    pub online: bool,
    pub error_count: u32,
    pub can_status: bool,
    pub metrics: HardwareMetrics,
}

#[derive(Debug, Clone)]
pub struct HardwareMetrics {
    pub cpu_usage: f32,
    pub memory_usage: f32,
    pub temperature: f32,
    pub uptime: Duration,
    pub last_update: SystemTime,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct HardwareParameter {
    pub id: u32,
    pub name: String,
    pub value: HardwareValue,
    pub unit: Option<String>,
}

#[derive(Debug, Serialize, Deserialize, Clone, PartialEq)]
#[serde(untagged)]
pub enum HardwareValue {
    Integer(i64),
    Float(f64),
    String(String),
    Boolean(bool),
}
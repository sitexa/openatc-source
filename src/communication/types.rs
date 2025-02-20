use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct CanMessage {
    pub id: u32,
    pub data: Vec<u8>,
    pub extended: bool,
    pub rtr: bool,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct CanConfig {
    pub interface: String,
    pub bitrate: u32,
    pub sample_point: f32,
}

#[derive(Debug, Clone)]
pub struct CanStatistics {
    pub messages_sent: u64,
    pub messages_received: u64,
    pub errors: u32,
    pub last_error_time: Option<std::time::SystemTime>,
}
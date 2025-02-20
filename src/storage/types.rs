use serde::{Deserialize, Serialize};
use std::time::SystemTime;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SignalData {
    pub timestamp: SystemTime,
    pub signal_id: u32,
    pub phase_states: Vec<PhaseStateData>,
    pub detector_states: Vec<DetectorData>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PhaseStateData {
    pub phase_id: u32,
    pub state: String,
    pub elapsed_time: u32,
    pub remaining_time: u32,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct DetectorData {
    pub detector_id: u32,
    pub occupancy: f32,
    pub volume: u32,
    pub speed: Option<f32>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct StorageConfig {
    pub data_dir: String,
    pub max_file_size: usize,
    pub max_history_days: u32,
    pub auto_cleanup: bool,
}
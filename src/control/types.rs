use serde::{Deserialize, Serialize};
use std::time::{Duration, SystemTime};
use crate::storage::{PhaseStateData, SignalData};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Phase {
    pub id: u32,
    pub name: String,
    pub movements: Vec<Movement>,
    pub min_green: Duration,
    pub max_green: Duration,
    pub yellow_time: Duration,
    pub red_clearance: Duration,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Movement {
    pub id: u32,
    pub direction: Direction,
    pub movement_type: MovementType,
    pub priority: u8,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum Direction {
    North,
    South,
    East,
    West,
    NorthEast,
    NorthWest,
    SouthEast,
    SouthWest,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum MovementType {
    Through,
    Left,
    Right,
    UTurn,
    Pedestrian,
}

#[derive(Debug, Clone, Copy, PartialEq, Serialize, Deserialize)]
pub enum PhaseState {
    Red,
    Yellow,
    Green,
    Flash,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PhaseStatus {
    pub phase_id: u32,
    pub state: PhaseState,
    pub remaining_time: Duration,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SignalStatus {
    pub signal_id: u32,
    pub phases: Vec<PhaseStatus>,
    pub current_plan: Option<TimingPlan>,
    pub mode: SignalMode,
}

#[derive(Debug, Clone, Copy, PartialEq, Serialize, Deserialize)]
pub enum SignalMode {
    Normal,
    Flash,
    AllRed,
    Manual,
    Coordination,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TimingPlan {
    pub id: u32,
    pub name: String,
    pub cycle_length: Duration,
    pub offset: Duration,
    pub phases: Vec<PhaseConfig>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PhaseConfig {
    pub phase_id: u32,
    pub split: Duration,
    pub phase_sequence: u32,
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum ControlMode {
    Fixed,      // 定周期控制
    Vehicle,    // 车辆响应
    Flash,      // 闪光
    AllRed,     // 全红
    Manual,     // 手动控制
    Error,      // 错误状态
}

impl ToString for ControlMode {
    fn to_string(&self) -> String {
        match self {
            ControlMode::Fixed => "Fixed".to_string(),
            ControlMode::Vehicle => "Vehicle".to_string(),
            ControlMode::Flash => "Flash".to_string(),
            ControlMode::AllRed => "AllRed".to_string(),
            ControlMode::Manual => "Manual".to_string(),
            ControlMode::Error => "Error".to_string(),
        }
    }
}

#[derive(Debug, Clone)]
pub struct ControllerState {
    pub current_plan: u32,
    pub current_phase: u32,
    pub phase_elapsed_time: u32,
    pub cycle_elapsed_time: u32,
    pub mode: ControlMode,
}

#[derive(Debug, Clone)]
pub struct VehicleDetection {
    pub detector_id: u32,
    pub timestamp: SystemTime,
    pub presence: bool,
    pub speed: Option<f32>,
    pub length: Option<f32>,
}

#[derive(Debug, Clone)]
pub struct ControlConfig {
    pub intersection_id: String,
    pub control_mode: ControlMode,
    pub cycle_length: u32,
}

impl From<ControllerState> for SignalData {
    fn from(state: ControllerState) -> Self {
        SignalData {
            timestamp: SystemTime::now(),
            signal_id: state.current_plan,
            phase_states: vec![PhaseStateData {
                phase_id: state.current_phase,
                state: match state.mode {
                    ControlMode::Fixed => "Fixed".to_string(),
                    ControlMode::Vehicle => "Vehicle".to_string(),
                    ControlMode::Flash => "Flash".to_string(),
                    ControlMode::AllRed => "AllRed".to_string(),
                    ControlMode::Manual => "Manual".to_string(),
                    ControlMode::Error => "Error".to_string(),
                },
                elapsed_time: state.phase_elapsed_time,
                remaining_time: 0, // TODO: Calculate remaining time
            }],
            detector_states: vec![], // TODO: Add detector states
        }
    }
}
use serde::{Deserialize, Serialize};
use std::time::Duration;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct IntersectionInfo {
    pub id: u32,
    pub name: String,
    pub location: GeoLocation,
    pub cycle_length: Duration,
    pub offset: Duration,
    pub coordination_phase: u32,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GeoLocation {
    pub latitude: f64,
    pub longitude: f64,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct CoordinationPlan {
    pub id: u32,
    pub name: String,
    pub cycle_length: Duration,
    pub intersections: Vec<IntersectionOffset>,
    pub green_wave_speed: f32,  // 米/秒
    pub direction: Direction,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct IntersectionOffset {
    pub intersection_id: u32,
    pub offset: Duration,
    pub split_ratio: f32,
}

#[derive(Debug, Clone, Copy, PartialEq, Serialize, Deserialize)]
pub enum Direction {
    NorthSouth,
    EastWest,
    Bidirectional,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GreenWaveBand {
    pub start_time: Duration,
    pub width: Duration,
    pub speed: f32,
    pub direction: Direction,
}
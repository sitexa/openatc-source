use serde::{Deserialize, Serialize};
use std::collections::HashMap;

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct HardwareConfig {
    pub device_id: String,
    pub device_type: DeviceType,
    pub can_interface: String,
    pub parameters: HashMap<String, Parameter>,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub enum DeviceType {
    TrafficLight,
    Detector,
    Controller,
    Other(String),
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct Parameter {
    pub id: u32,
    pub name: String,
    pub value: ParameterValue,
    pub unit: Option<String>,
}

#[derive(Debug, Serialize, Deserialize, Clone, PartialEq)]
#[serde(untagged)]
pub enum ParameterValue {
    Integer(i64),
    Float(f64),
    String(String),
    Boolean(bool),
}
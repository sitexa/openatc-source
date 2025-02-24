use super::HardwareError;
use super::HardwareResult;
use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::fs;
use std::time::{Duration, SystemTime};

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct HardwareConfig {
    pub hardware_id: String,
    pub hardware_type: HardwareType,
    pub can_interface: String,
    pub parameters: HashMap<String, HardwareParameter>,
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
    pub description: Option<String>,
}

#[derive(Debug, Serialize, Deserialize, Clone, PartialEq)]
#[serde(untagged)]
pub enum HardwareValue {
    Integer(i64),
    Float(f64),
    String(String),
    Boolean(bool),
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


#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct ParameterStore {
    pub(crate) parameters: HashMap<String, HardwareParameter>,
    definitions: HashMap<String, HardwareParameter>,
}

impl ParameterStore {
    pub fn new() -> Self {
        Self {
            parameters: HashMap::new(),
            definitions: HashMap::new(),
        }
    }

    pub fn validate_parameter(&self, name: &str, value: &HardwareValue) -> HardwareResult<()> {
        if let Some(def) = self.definitions.get(name) {
            if std::mem::discriminant(&def.value) != std::mem::discriminant(value) {
                return Err(HardwareError::ParameterError(
                    format!("参数类型不匹配: {}", name)
                ));
            }
            Ok(())
        } else {
            Err(HardwareError::ParameterError(format!("参数未定义: {}", name)))
        }
    }

    pub fn set_value(&mut self, name: &str, value: HardwareValue) -> HardwareResult<()> {
        if let Some(param) = self.parameters.get_mut(name) {
            param.value = value;
            Ok(())
        } else {
            Err(HardwareError::ParameterError(format!("参数不存在: {}", name)))
        }
    }

    pub fn get_parameter_definition(&self, name: &str) -> Option<&HardwareParameter> {
        self.definitions.get(name)
    }

    pub fn save_to_file(&self, path: &str) -> HardwareResult<()> {
        let json = serde_json::to_string_pretty(self)
            .map_err(|e| HardwareError::ParameterError(e.to_string()))?;
        fs::write(path, json)
            .map_err(|e| HardwareError::ParameterError(e.to_string()))?;
        Ok(())
    }

    pub fn load_from_file(path: &str) -> HardwareResult<Self> {
        let content = fs::read_to_string(path)
            .map_err(|e| HardwareError::ParameterError(e.to_string()))?;
        serde_json::from_str(&content)
            .map_err(|e| HardwareError::ParameterError(e.to_string()))
    }
}
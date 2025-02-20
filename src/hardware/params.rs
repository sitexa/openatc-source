use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use tokio::sync::{broadcast, Mutex};
use std::sync::Arc;
use std::fs;
use std::time::SystemTime;
use tracing::warn;
use crate::hardware::{HardwareError, ParameterValue};
use super::error::HardwareResult;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ParameterDefinition {
    pub id: u32,
    pub name: String,
    pub description: String,
    pub data_type: ParameterType,
    pub min_value: Option<f64>,
    pub max_value: Option<f64>,
    pub default_value: ParameterValue,
    pub unit: Option<String>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum ParameterType {
    Integer,
    Float,
    Boolean,
    String,
}

#[derive(Debug, Clone)]
pub struct ParameterChangeEvent {
    pub name: String,
    pub old_value: Option<ParameterValue>,
    pub new_value: ParameterValue,
    pub timestamp: std::time::SystemTime,
}


#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct StoredParameters {
    parameters: HashMap<String, ParameterDefinition>,
    values: HashMap<String, ParameterValue>,
    value_timestamps: HashMap<String, SystemTime>,
}

pub struct ParameterStore {
    stored: StoredParameters,
    change_sender: broadcast::Sender<ParameterChangeEvent>,
}


impl ParameterStore {
    pub fn new() -> Self {
        let (sender, _) = broadcast::channel(100);
        Self {
            stored: StoredParameters {
                parameters: HashMap::new(),
                values: HashMap::new(),
                value_timestamps: HashMap::new(),
            },
            change_sender: sender,
        }
    }

    pub fn subscribe(&self) -> broadcast::Receiver<ParameterChangeEvent> {
        self.change_sender.subscribe()
    }

    pub fn validate_parameter(&self, name: &str, value: &ParameterValue) -> HardwareResult<()> {
        if let Some(def) = self.stored.parameters.get(name) {
            match (&def.data_type, value) {
                (ParameterType::Integer, ParameterValue::Integer(v)) => {
                    if let (Some(min), Some(max)) = (def.min_value, def.max_value) {
                        if (*v as f64) < min || (*v as f64) > max {
                            return Err(HardwareError::ParameterError(
                                format!("参数值超出范围: {} ({} - {})", name, min, max)
                            ));
                        }
                    }
                }
                (ParameterType::Float, ParameterValue::Float(v)) => {
                    if let (Some(min), Some(max)) = (def.min_value, def.max_value) {
                        if *v < min || *v > max {
                            return Err(HardwareError::ParameterError(
                                format!("参数值超出范围: {} ({} - {})", name, min, max)
                            ));
                        }
                    }
                }
                _ => return Err(HardwareError::ParameterError(
                    format!("参数类型不匹配: {}", name)
                )),
            }
            Ok(())
        } else {
            Err(HardwareError::ParameterError(format!("参数未定义: {}", name)))
        }
    }

    pub fn save_to_file(&self, path: &str) -> HardwareResult<()> {
        let json = serde_json::to_string_pretty(&self.stored)
            .map_err(|e| HardwareError::StorageError(e.to_string()))?;
        fs::write(path, json)
            .map_err(|e| HardwareError::StorageError(e.to_string()))?;
        Ok(())
    }

    pub fn load_from_file(path: &str) -> HardwareResult<Self> {
        let json = fs::read_to_string(path)
            .map_err(|e| HardwareError::StorageError(e.to_string()))?;
        let stored: StoredParameters = serde_json::from_str(&json)
            .map_err(|e| HardwareError::StorageError(e.to_string()))?;
        let (sender, _) = broadcast::channel(100);
        
        Ok(Self {
            stored,
            change_sender: sender,
        })
    }

    pub fn get_parameter_definition(&self, name: &str) -> Option<&ParameterDefinition> {
        self.stored.parameters.get(name)
    }

    pub fn set_value(&mut self, name: &str, value: ParameterValue) -> HardwareResult<()> {
        if self.stored.parameters.contains_key(name) {
            let old_value = self.stored.values.get(name).cloned();
            self.stored.values.insert(name.to_string(), value.clone());
            self.stored.value_timestamps.insert(name.to_string(), SystemTime::now());

            let event = ParameterChangeEvent {
                name: name.to_string(),
                old_value,
                new_value: value,
                timestamp: SystemTime::now(),
            };

            if let Err(e) = self.change_sender.send(event) {
                warn!("发送参数变更通知失败: {}", e);
            }
            Ok(())
        } else {
            Err(HardwareError::ParameterError(format!("参数未定义: {}", name)))
        }
    }

    pub fn get_value(&self, name: &str) -> Option<&ParameterValue> {
        self.stored.values.get(name)
    }

    pub fn get_all_values(&self) -> Vec<(String, ParameterValue)> {
        self.stored.values.iter()
            .map(|(k, v)| (k.clone(), v.clone()))
            .collect()
    }

    pub fn get_value_timestamp(&self, name: &str) -> Option<SystemTime> {
        self.stored.value_timestamps.get(name).cloned()
    }
}
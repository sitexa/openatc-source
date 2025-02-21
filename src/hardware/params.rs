use std::collections::HashMap;
use serde::{Serialize, Deserialize};
use std::fs;
use crate::hardware::error::{HardwareError, HardwareResult};
use crate::hardware::types::HardwareValue;

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct HardwareParameter {
    pub id: u32,
    pub name: String,
    pub value: HardwareValue,
    pub unit: Option<String>,
    pub description: Option<String>,
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
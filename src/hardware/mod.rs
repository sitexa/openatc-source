mod error;
mod types;
mod status;
mod params;
mod monitor;
mod manager;
mod sync;

#[cfg(test)]
mod tests;

pub use error::{HardwareError, HardwareResult};
pub use types::{
    HardwareConfig,
    HardwareType,  // Changed from DeviceType
    HardwareValue, // Changed from ParameterValue
    HardwareParameter,
};
pub use status::{HardwareStatus, HardwareMetrics};
pub use params::ParameterStore;
pub use monitor::HardwareMonitor;
pub use manager::HardwareManager;
mod error;
pub mod types;
mod manager;
mod monitor;
mod params;
mod sync;

pub use error::{HardwareError, HardwareResult};
pub use manager::HardwareManager;
pub use types::*;
pub use monitor::{DeviceMonitor, DeviceStatus, PerformanceMetrics};
pub use params::{ParameterStore, ParameterDefinition, ParameterType, ParameterChangeEvent};
pub use sync::{ParameterSynchronizer, SyncState, ParameterSyncStatus};
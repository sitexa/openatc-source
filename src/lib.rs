pub mod hardware;
pub mod control;
pub mod communication;
pub mod fault;
pub mod config;
pub mod storage;
pub mod monitor;
pub mod coordination;
pub mod interface;
pub mod optimization;

// 重导出常用类型
pub use hardware::{HardwareManager, HardwareError};
pub use communication::{CanConnection, CommError};
pub use control::{ControlManager, ControlError};
pub use fault::{FaultManager, FaultError};
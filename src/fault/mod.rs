pub mod types;
mod error;
mod manager;

pub use error::{FaultError, FaultResult};
pub use manager::FaultManager;
pub use types::*;

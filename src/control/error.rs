use thiserror::Error;

#[derive(Error, Debug)]
pub enum ControlError {
    #[error("配时方案错误: {0}")]
    TimingError(String),

    #[error("相位错误: {0}")]
    PhaseError(String),

    #[error("模式切换错误: {0}")]
    ModeChangeError(String),

    #[error("状态错误: {0}")]
    StateError(String),

    #[error("硬件通信错误: {0}")]
    HardwareError(String),

    #[error("验证错误: {0}")]
    ValidationError(String),
}

pub type ControlResult<T> = Result<T, ControlError>;
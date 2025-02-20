use thiserror::Error;

#[derive(Error, Debug)]
pub enum FaultError {
    #[error("故障未找到: {0}")]
    NotFound(String),

    #[error("存储错误: {0}")]
    StorageError(String),

    #[error("配置错误: {0}")]
    ConfigError(String),
}

pub type FaultResult<T> = Result<T, FaultError>;
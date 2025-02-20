use thiserror::Error;

#[derive(Error, Debug)]
pub enum StorageError {
    #[error("IO错误: {0}")]
    IoError(String),

    #[error("序列化错误: {0}")]
    SerializeError(String),

    #[error("查询错误: {0}")]
    QueryError(String),

    #[error("配置错误: {0}")]
    ConfigError(String),
}

pub type StorageResult<T> = Result<T, StorageError>;
use thiserror::Error;

#[derive(Error, Debug)]
pub enum HardwareError {
    #[error("设备初始化失败: {0}")]
    InitializationError(String),

    #[error("参数错误: {0}")]
    ParameterError(String),

    #[error("通信错误: {0}")]
    CommunicationError(String),

    #[error("设备未找到: {0}")]
    DeviceNotFound(String),

    #[error("写文件错误: {0}")]
    StorageError(String),

    #[error("操作超时")]
    Timeout,

}

pub type HardwareResult<T> = Result<T, HardwareError>;
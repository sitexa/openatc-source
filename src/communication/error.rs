use thiserror::Error;

#[derive(Error, Debug)]
pub enum CommError {
    #[error("CAN总线错误: {0}")]
    CanBusError(String),

    #[error("连接错误: {0}")]
    ConnectionError(String),

    #[error("消息格式错误: {0}")]
    MessageFormatError(String),

    #[error("CAN socket error: {0}")]
    SocketError(String),

    #[error("Buffer error: {0}")]
    BufferError(String),

    #[error("Configuration error: {0}")]
    ConfigError(String),

    #[error("模拟通信错误: {0}")]
    SendError(String),

    #[error("发送超时")]
    SendTimeout,

    #[error("接收超时")]
    ReceiveTimeout,
}


pub type CommResult<T> = Result<T, CommError>;
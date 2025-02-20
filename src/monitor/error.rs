use thiserror::Error;

#[derive(Error, Debug)]
pub enum MonitorError {
    #[error("系统错误: {0}")]
    SystemError(String),

    #[error("度量错误: {0}")]
    MetricError(String),

    #[error("网络监控错误: {0}")]
    NetworkError(String),
}

pub type MonitorResult<T> = Result<T, MonitorError>;

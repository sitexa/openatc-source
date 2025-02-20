use std::time::{Duration, SystemTime};
use tokio::sync::Mutex;
use std::sync::Arc;
use tracing::{info, warn, error};
use super::error::HardwareResult;
use sysinfo::{System, SystemExt, CpuExt};

#[derive(Debug, Clone)]
pub struct DeviceStatus {
    pub device_id: String,
    pub online: bool,
    pub last_heartbeat: SystemTime,
    pub performance_metrics: PerformanceMetrics,
}

#[derive(Debug, Clone)]
pub struct PerformanceMetrics {
    pub cpu_usage: f32,
    pub memory_usage: f32,
    pub temperature: f32,
    pub uptime: Duration,
    pub last_update: SystemTime,
}

#[derive(Clone)]
pub struct DeviceMonitor {
    status: Arc<Mutex<DeviceStatus>>,
    heartbeat_interval: Duration,
    offline_threshold: Duration,
}

impl DeviceMonitor {
    pub fn new(device_id: String) -> Self {
        Self {
            status: Arc::new(Mutex::new(DeviceStatus {
                device_id,
                online: false,
                last_heartbeat: SystemTime::now(),
                performance_metrics: PerformanceMetrics {
                    cpu_usage: 0.0,
                    memory_usage: 0.0,
                    temperature: 0.0,
                    uptime: Duration::from_secs(0),
                    last_update: SystemTime::now(),
                },
            })),
            heartbeat_interval: Duration::from_secs(1),
            offline_threshold: Duration::from_secs(5),
        }
    }

    pub async fn start_monitoring(&self) -> HardwareResult<()> {
        let monitor = self.clone();
        
        tokio::spawn(async move {
            loop {
                let mut status = monitor.status.lock().await;
                let now = SystemTime::now();
                
                if let Ok(duration) = now.duration_since(status.last_heartbeat) {
                    if duration > monitor.offline_threshold {
                        if status.online {
                            status.online = false;
                            warn!("设备离线: {}", status.device_id);
                        }
                    }
                }
                
                drop(status);
                tokio::time::sleep(Duration::from_secs(1)).await;
            }
        });
        
        Ok(())
    }

    pub async fn update_heartbeat(&self) -> HardwareResult<()> {
        let mut status = self.status.lock().await;
        status.last_heartbeat = SystemTime::now();
        if !status.online {
            status.online = true;
            info!("设备上线: {}", status.device_id);
        }
        Ok(())
    }

    pub async fn collect_metrics(&self) -> HardwareResult<PerformanceMetrics> {
        let mut sys = System::new_all();
        sys.refresh_all();

        let cpu_usage = sys.global_cpu_info().cpu_usage();
        let total_memory = sys.total_memory() as f32;
        let used_memory = sys.used_memory() as f32;
        let memory_usage = (used_memory / total_memory) * 100.0;
        let uptime = Duration::from_secs(sys.uptime());

        let metrics = PerformanceMetrics {
            cpu_usage,
            memory_usage,
            temperature: self.read_temperature().await?,
            uptime,
            last_update: SystemTime::now(),
        };

        self.update_metrics(metrics.clone()).await?;
        Ok(metrics)
    }

    async fn read_temperature(&self) -> HardwareResult<f32> {
        // 通过硬件接口读取温度数据
        Ok(45.0)
    }

    pub async fn start_metrics_collection(&self) -> HardwareResult<()> {
        let monitor = self.clone();

        tokio::spawn(async move {
            loop {
                if let Err(e) = monitor.collect_metrics().await {
                    error!("性能指标采集错误: {}", e);
                }
                tokio::time::sleep(Duration::from_secs(5)).await;
            }
        });

        Ok(())
    }

    pub async fn update_metrics(&self, metrics: PerformanceMetrics) -> HardwareResult<()> {
        let mut status = self.status.lock().await;
        status.performance_metrics = metrics;
        Ok(())
    }
}
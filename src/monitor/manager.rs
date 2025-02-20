use super::types::*;
use super::error::{MonitorError, MonitorResult};
use tokio::sync::Mutex;
use std::sync::Arc;
use std::time::{SystemTime, Duration};
use sysinfo::{System, SystemExt, ProcessExt, CpuExt};
use tracing::{info, warn, error};

pub struct MonitorManager {
    system: Arc<Mutex<System>>,
    start_time: SystemTime,
    metrics: Arc<Mutex<Vec<PerformanceMetrics>>>,
    network_stats: Arc<Mutex<NetworkStatus>>,
}

impl MonitorManager {
    pub fn new() -> Self {
        Self {
            system: Arc::new(Mutex::new(System::new_all())),
            start_time: SystemTime::now(),
            metrics: Arc::new(Mutex::new(Vec::new())),
            network_stats: Arc::new(Mutex::new(NetworkStatus {
                can_connected: false,
                message_rate: 0.0,
                error_count: 0,
            })),
        }
    }

    pub async fn update(&self) -> MonitorResult<()> {
        let mut sys = self.system.lock().await;
        sys.refresh_all();

        // 更新系统状态
        let status = SystemStatus {
            timestamp: SystemTime::now(),
            cpu_usage: sys.global_cpu_info().cpu_usage(),
            memory_usage: sys.used_memory() as f32 / sys.total_memory() as f32,
            uptime: SystemTime::now().duration_since(self.start_time)
                .unwrap_or(Duration::from_secs(0)),
            process_status: self.get_process_status().await?,
            network_status: self.network_stats.lock().await.clone(),
        };

        info!("系统状态更新: CPU使用率 {:.1}%, 内存使用率 {:.1}%", 
            status.cpu_usage, status.memory_usage * 100.0);

        if status.cpu_usage > 80.0 || status.memory_usage > 0.8 {
            warn!("系统资源使用率过高");
        }

        Ok(())
    }

    pub async fn record_metric(&self, metric: PerformanceMetrics) -> MonitorResult<()> {
        let mut metrics = self.metrics.lock().await;
        metrics.push(metric);

        // 保持最近1000条记录
        if metrics.len() > 1000 {
            metrics.remove(0);
        }

        Ok(())
    }

    pub async fn get_average_metrics(&self) -> MonitorResult<Option<PerformanceMetrics>> {
        let metrics = self.metrics.lock().await;
        if metrics.is_empty() {
            return Ok(None);
        }

        let count = metrics.len() as f32;
        let avg_cycle = metrics.iter()
            .map(|m| m.cycle_time.as_micros() as f32)
            .sum::<f32>() / count;
        let avg_response = metrics.iter()
            .map(|m| m.response_time.as_micros() as f32)
            .sum::<f32>() / count;
        let avg_queue = metrics.iter()
            .map(|m| m.queue_length as f32)
            .sum::<f32>() / count;

        Ok(Some(PerformanceMetrics {
            cycle_time: Duration::from_micros(avg_cycle as u64),
            response_time: Duration::from_micros(avg_response as u64),
            queue_length: avg_queue as usize,
        }))
    }

    async fn get_process_status(&self) -> MonitorResult<ProcessStatus> {
        Ok(ProcessStatus {
            thread_count: std::thread::available_parallelism()
                .map(|n| n.get())
                .unwrap_or(1),
            open_files: 0, // TODO: 实现文件句柄统计
            last_gc_time: SystemTime::now(),
        })
    }

    pub async fn update_network_status(&self, connected: bool, rate: f32, errors: u32) {
        let mut stats = self.network_stats.lock().await;
        stats.can_connected = connected;
        stats.message_rate = rate;
        stats.error_count = errors;
    }
}
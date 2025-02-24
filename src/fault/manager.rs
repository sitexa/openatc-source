use super::{error::{FaultError, FaultResult}, types::*};
use tokio::sync::Mutex;
use std::sync::Arc;
use std::collections::HashMap;
use std::time::SystemTime;
use tracing::{info, error, warn};

pub struct FaultManager {
    config: Arc<Mutex<FaultConfig>>,
    active_faults: Arc<Mutex<HashMap<u32, FaultEvent>>>,
    fault_history: Arc<Mutex<Vec<FaultEvent>>>,
    next_fault_id: Arc<Mutex<u32>>,
}

impl FaultManager {
    pub fn new(config: FaultConfig) -> Self {
        Self {
            config: Arc::new(Mutex::new(config)),
            active_faults: Arc::new(Mutex::new(HashMap::new())),
            fault_history: Arc::new(Mutex::new(Vec::new())),
            next_fault_id: Arc::new(Mutex::new(1)),
        }
    }

    pub async fn report_fault(&self,
                              fault_type: FaultType,
                              severity: FaultSeverity,
                              description: String,
    ) -> FaultResult<u32> {
        let mut id = self.next_fault_id.lock().await;
        let fault_id = *id;
        *id += 1;

        let event = FaultEvent {
            id: fault_id,
            fault_type,
            severity,
            description,
            timestamp: SystemTime::now(),
            resolved: false,
            resolution_time: None,
        };

        let mut active = self.active_faults.lock().await;
        active.insert(fault_id, event.clone());

        if severity == FaultSeverity::Critical {
            error!("严重故障: [{}] {}", fault_id, event.description);
        } else {
            warn!("故障报告: [{}] {}", fault_id, event.description);
        }

        Ok(fault_id)
    }

    pub async fn get_active_faults(&self) -> Vec<FaultEvent> {
        let active = self.active_faults.lock().await;
        active.values().cloned().collect()
    }

    pub async fn get_fault_history(&self) -> Vec<FaultEvent> {
        let history = self.fault_history.lock().await;
        history.clone()
    }

    pub async fn resolve_fault(&self, fault_id: u32) -> FaultResult<()> {
        let mut active = self.active_faults.lock().await;

        if let Some(mut fault) = active.remove(&fault_id) {
            fault.resolved = true;
            fault.resolution_time = Some(SystemTime::now());

            let mut history = self.fault_history.lock().await;
            history.push(fault.clone());

            info!("故障已解决: [{}] {}", fault_id, fault.description);
            Ok(())
        } else {
            Err(FaultError::NotFound(format!("故障ID不存在: {}", fault_id)))
        }
    }

    pub async fn clear_history(&self) -> FaultResult<()> {
        let mut history = self.fault_history.lock().await;
        history.clear();
        info!("故障历史记录已清除");
        Ok(())
    }

    pub async fn monitor_faults(&self) -> FaultResult<()> {
        let config = self.config.lock().await;
        let active = self.active_faults.lock().await;

        // 检查自动解决超时
        if let Some(timeout) = config.auto_resolve_timeout {
            let faults_to_resolve: Vec<(u32, String)> = active.iter()
                .filter(|(_, fault)| !fault.resolved)
                .filter_map(|(id, fault)| {
                    if let Ok(elapsed) = fault.timestamp.elapsed() {
                        if elapsed >= timeout {
                            Some((*id, fault.description.clone()))
                        } else {
                            None
                        }
                    } else {
                        None
                    }
                })
                .collect();

            // 释放锁后再处理需要解决的故障
            drop(active);
            
            for (fault_id, description) in faults_to_resolve {
                self.resolve_fault(fault_id).await?;
                info!("故障自动解决: [{}] {}", fault_id, description);
            }
        }

        Ok(())
    }
}
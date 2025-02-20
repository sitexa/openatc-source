use std::cmp::PartialEq;
use super::error::HardwareResult;
use super::params::{ParameterStore};
use super::types::{ParameterValue};
use std::collections::HashMap;
use std::time::{SystemTime, Duration};
use tokio::sync::Mutex;
use std::sync::Arc;
use tracing::{info, warn, error};

#[derive(Debug, Clone)]
pub struct ParameterSyncStatus {
    pub last_sync: SystemTime,
    pub sync_state: SyncState,
    pub retry_count: u32,
}

#[derive(Debug, Clone, PartialEq)]
pub enum SyncState {
    Synced,
    Pending,
    Failed,
    Conflict,
}

#[derive(Clone)]
pub struct ParameterSynchronizer {
    store: Arc<Mutex<ParameterStore>>,
    sync_status: Arc<Mutex<HashMap<String, ParameterSyncStatus>>>,
    sync_interval: Duration,
    max_retries: u32,
}


impl ParameterSynchronizer {
    pub fn new(store: Arc<Mutex<ParameterStore>>) -> Self {
        Self {
            store,
            sync_status: Arc::new(Mutex::new(HashMap::new())),
            sync_interval: Duration::from_secs(60),
            max_retries: 3,
        }
    }

    pub async fn start_sync(&self) -> HardwareResult<()> {
        let synchronizer = self.clone();

        tokio::spawn(async move {
            loop {
                if let Err(e) = synchronizer.sync_all_parameters().await {
                    error!("参数同步错误: {}", e);
                }
                tokio::time::sleep(synchronizer.sync_interval).await;
            }
        });

        Ok(())
    }

    pub async fn sync_all_parameters(&self) -> HardwareResult<()> {
        let store = self.store.lock().await;
        let mut sync_status = self.sync_status.lock().await;

        for (name, value) in store.get_all_values() {
            let status = sync_status.entry(name.clone())
                .or_insert(ParameterSyncStatus {
                    last_sync: SystemTime::now(),
                    sync_state: SyncState::Pending,
                    retry_count: 0,
                });

            if status.sync_state == SyncState::Failed && status.retry_count >= self.max_retries {
                warn!("参数同步失败次数过多: {}", name);
                continue;
            }

            if let Err(e) = self.sync_parameter(&name, &value).await {
                status.sync_state = SyncState::Failed;
                status.retry_count += 1;
                error!("参数同步失败: {} - {}", name, e);
            } else {
                status.sync_state = SyncState::Synced;
                status.retry_count = 0;
                status.last_sync = SystemTime::now();
            }
        }

        Ok(())
    }

    async fn sync_parameter(&self, name: &str, value: &ParameterValue) -> HardwareResult<()> {
        // 获取设备当前参数值
        let device_value = self.get_device_parameter(name).await?;

        match device_value {
            Some(dev_value) => {
                if dev_value != *value {
                    // 处理参数冲突
                    self.handle_parameter_conflict(name, value, &dev_value).await?;
                }
            }
            None => {
                // 设备没有该参数，直接更新
                self.update_device_parameter(name, value).await?;
            }
        }

        Ok(())
    }

    async fn get_device_parameter(&self, name: &str) -> HardwareResult<Option<ParameterValue>> {
        // 从设备读取参数值的实现
        Ok(None)
    }

    async fn update_device_parameter(&self, name: &str, value: &ParameterValue) -> HardwareResult<()> {
        // 更新设备参数值的实现
        Ok(())
    }

    async fn handle_parameter_conflict(
        &self,
        name: &str,
        local_value: &ParameterValue,
        device_value: &ParameterValue,
    ) -> HardwareResult<()> {
        let mut sync_status = self.sync_status.lock().await;
        let status = sync_status.get_mut(name).unwrap();
        status.sync_state = SyncState::Conflict;

        // 根据时间戳决定使用哪个值
        let mut store = self.store.lock().await;
        if let Some(local_timestamp) = store.get_value_timestamp(name) {
            if local_timestamp > status.last_sync {
                // 本地值更新，更新设备
                self.update_device_parameter(name, local_value).await?;
            } else {
                // 设备值更新，更新本地
                store.set_value(name, device_value.clone())?;
            }
        }

        Ok(())
    }
}
use super::error::{StorageError, StorageResult};
use super::types::*;
use chrono::{DateTime, Local};
use std::fs::{self, File};
use std::io::Write;
use std::path::Path;
use std::sync::Arc;
use std::time::SystemTime;
use tokio::sync::Mutex;
use tracing::{error, info, warn};

pub struct StorageManager {
    config: Arc<Mutex<StorageConfig>>,
    current_file: Arc<Mutex<Option<File>>>,
    current_size: Arc<Mutex<usize>>,
}

impl StorageManager {
    pub fn new(config: StorageConfig) -> StorageResult<Self> {
        fs::create_dir_all(&config.data_dir)
            .map_err(|e| StorageError::IoError(e.to_string()))?;

        Ok(Self {
            config: Arc::new(Mutex::new(config)),
            current_file: Arc::new(Mutex::new(None)),
            current_size: Arc::new(Mutex::new(0)),
        })
    }

    pub async fn store_signal_data(&self, data: SignalData) -> StorageResult<()> {
        let json = serde_json::to_string(&data)
            .map_err(|e| StorageError::SerializeError(e.to_string()))?;

        let mut current_file = self.current_file.lock().await;
        let mut current_size = self.current_size.lock().await;

        // 检查是否需要创建新文件
        if current_file.is_none() || *current_size >= self.config.lock().await.max_file_size {
            self.rotate_file(&mut current_file).await?;
            *current_size = 0;
        }

        // 写入数据
        if let Some(file) = current_file.as_mut() {
            writeln!(file, "{}", json)
                .map_err(|e| StorageError::IoError(e.to_string()))?;
            *current_size += json.len() + 1;
        }

        Ok(())
    }

    async fn rotate_file(&self, current_file: &mut Option<File>) -> StorageResult<()> {
        let config = self.config.lock().await;
        let timestamp = Local::now();
        let filename = format!("signal_data_{}.json", timestamp.format("%Y%m%d_%H%M%S"));
        let path = Path::new(&config.data_dir).join(filename);

        let file = File::create(path)
            .map_err(|e| StorageError::IoError(e.to_string()))?;

        *current_file = Some(file);

        // 清理旧文件
        if config.auto_cleanup {
            self.cleanup_old_files().await?;
        }

        Ok(())
    }

    async fn cleanup_old_files(&self) -> StorageResult<()> {
        let config = self.config.lock().await;
        let max_age = chrono::Duration::days(config.max_history_days as i64);

        let entries = fs::read_dir(&config.data_dir)
            .map_err(|e| StorageError::IoError(e.to_string()))?;

        for entry in entries {
            if let Ok(entry) = entry {
                if let Ok(metadata) = entry.metadata() {
                    if let Ok(modified) = metadata.modified() {
                        let modified: DateTime<Local> = modified.into();
                        if Local::now() - modified > max_age {
                            if let Err(e) = fs::remove_file(entry.path()) {
                                warn!("删除旧文件失败: {:?} - {}", entry.path(), e);
                            }
                        }
                    }
                }
            }
        }

        Ok(())
    }

    pub async fn query_data(&self, start_time: SystemTime, end_time: SystemTime) -> StorageResult<Vec<SignalData>> {
        // TODO: 实现数据查询功能
        Ok(Vec::new())
    }
}
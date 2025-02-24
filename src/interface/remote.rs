use super::types::*;
use std::time::Duration;
use tokio::sync::mpsc;
use tracing::{error, info};

pub struct RemoteController {
    command_tx: mpsc::Sender<RemoteCommand>,
    heartbeat_interval: Duration,
}

impl RemoteController {
    pub fn new(command_tx: mpsc::Sender<RemoteCommand>) -> Self {
        Self {
            command_tx,
            heartbeat_interval: Duration::from_secs(5),
        }
    }

    pub async fn start_heartbeat(&self) {
        let tx = self.command_tx.clone();
        let interval = self.heartbeat_interval;

        tokio::spawn(async move {
            loop {
                // 发送心跳包
                tokio::time::sleep(interval).await;

                // 发送心跳命令
                let heartbeat = RemoteCommand {
                    command_type: CommandType::Heartbeat,
                    intersection_id: 0,
                    parameters: serde_json::Value::Null,
                };

                if let Err(e) = tx.send(heartbeat).await {
                    error!("心跳包发送失败: {}", e);
                    break;
                }
            }
        });
    }

    pub async fn send_command(&self, command: RemoteCommand) -> Result<(), String> {
        self.command_tx.send(command).await
            .map_err(|e| format!("发送命令失败: {}", e))
    }
}
use tokio::sync::mpsc;
use tracing::info;
use crate::common::setup::MockMessage;

pub async fn monitor_mock_messages() -> mpsc::Receiver<MockMessage> {
    let (tx, rx) = mpsc::channel(100);
    
    tokio::spawn(async move {
        while let Some(msg) = rx.recv().await {
            info!("Mock CAN消息: ID={:03X}, Data={:?}", msg.id, msg.data);
        }
    });

    rx
}
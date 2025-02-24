use std::sync::{Arc};
use tokio::sync::{mpsc, Mutex};
use tracing::info;
use traeatc::communication::types::CanMessage;

pub fn create_mock_monitor() -> (mpsc::Sender<CanMessage>, Arc<Mutex<mpsc::Receiver<CanMessage>>>) {
    let (tx, rx) = mpsc::channel::<CanMessage>(100);
    let rx = Arc::new(Mutex::new(rx));
    let rx_clone = Arc::clone(&rx);

    tokio::spawn(async move {
        loop {
            let mut guard = rx_clone.lock().await;
            if let Some(msg) = guard.recv().await {
                info!("Mock CAN消息: ID={:03X}, Data={:?}", msg.id, msg.data);
            }
        }
    });

    (tx, rx)
}
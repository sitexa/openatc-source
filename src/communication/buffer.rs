use tokio::sync::mpsc;
use super::types::CanMessage;
use std::time::Duration;

pub struct MessageBuffer {
    tx: mpsc::Sender<CanMessage>,
    rx: mpsc::Receiver<CanMessage>,
    capacity: usize,
}

impl MessageBuffer {
    pub fn new(capacity: usize) -> Self {
        let (tx, rx) = mpsc::channel(capacity);
        Self {
            tx,
            rx,
            capacity,
        }
    }

    pub async fn send(&self, message: CanMessage) -> Result<(), mpsc::error::SendError<CanMessage>> {
        self.tx.send(message).await
    }

    pub async fn receive(&mut self) -> Option<CanMessage> {
        self.rx.recv().await
    }

    pub async fn receive_timeout(&mut self, timeout: Duration) -> Option<CanMessage> {
        tokio::time::timeout(timeout, self.rx.recv()).await.ok().flatten()
    }
}
use super::{types::*, filter::MessageFilter, priority::PriorityQueue, buffer::MessageBuffer, CommResult,CommError};
use tokio::sync::Mutex;
use std::sync::Arc;
use std::time::Duration;
use tracing::{info, warn, error};

pub struct CanConnection {
    config: CanConfig,
    filter: Arc<Mutex<MessageFilter>>,
    priority_queue: Arc<Mutex<PriorityQueue>>,
    buffer: Arc<MessageBuffer>,
    socket: Arc<Mutex<socketcan::CANSocket>>,
}

impl CanConnection {
    pub async fn new(interface: &str) -> CommResult<Self> {
        let socket = socketcan::CANSocket::open(interface).map_err(|e| CommError::SocketError(e.to_string()))?;
        socket.set_nonblocking(true)
            .map_err(|e| CommError::SocketError(e.to_string()))?;

        Ok(Self {
            config: CanConfig {
                interface: interface.to_string(),
                bitrate: 500_000,
                sample_point: 0.875,
            },
            filter: Arc::new(Mutex::new(MessageFilter::new())),
            priority_queue: Arc::new(Mutex::new(PriorityQueue::new())),
            buffer: Arc::new(MessageBuffer::new(1000)),
            socket: Arc::new(Mutex::new(socket)),
        })
    }

    pub async fn send_message(&self, message: CanMessage) -> CommResult<()> {
        let mut queue = self.priority_queue.lock().await;
        let priority = self.get_message_priority(&message);
        queue.push(message, priority);
        self.process_queue().await
    }

    async fn process_queue(&self) -> CommResult<()> {
        let mut queue = self.priority_queue.lock().await;
        while let Some(message) = queue.pop() {
            let socket = self.socket.lock().await;
            let frame = socketcan::CANFrame::new(
                message.id,
                &message.data,
                message.rtr,
                message.extended,
            ).map_err(|e| CommError::SocketError(e.to_string()))?;

            socket.write_frame(&frame)
                .map_err(|e| CommError::SocketError(e.to_string()))?;
        }
        Ok(())
    }

    fn get_message_priority(&self, message: &CanMessage) -> u8 {
        // 基于消息 ID 计算优先级
        (message.id >> 24) as u8
    }
}
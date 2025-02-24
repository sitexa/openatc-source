use super::{types::*, filter::MessageFilter, priority::PriorityQueue, buffer::MessageBuffer, CommResult, CommError};
use tokio::sync::{Mutex, mpsc};
use std::sync::Arc;
use tracing::{info, warn, error};

pub struct CanConnection {
    config: CanConfig,
    filter: Arc<Mutex<MessageFilter>>,
    priority_queue: Arc<Mutex<PriorityQueue>>,
    buffer: Arc<MessageBuffer>,
    socket: Arc<Mutex<Box<dyn CanSocket + Send>>>,
}

// CAN 接口抽象
#[async_trait::async_trait]
pub trait CanSocket {
    async fn write_frame(&self, frame: &CanMessage) -> CommResult<()>;
    async fn read_frame(&self) -> CommResult<CanMessage>;
}

// 真实 CAN 设备实现
struct RealCanSocket {
    socket: socketcan::CANSocket,
}

#[async_trait::async_trait]
impl CanSocket for RealCanSocket {
    async fn write_frame(&self, frame: &CanMessage) -> CommResult<()> {
        let can_frame = socketcan::CANFrame::new(
            frame.id,
            &frame.data,
            frame.rtr,
            frame.extended
        ).map_err(|e| CommError::SocketError(e.to_string()))?;
        
        self.socket.write_frame(&can_frame)
            .map_err(|e| CommError::SocketError(e.to_string()))
    }

    async fn read_frame(&self) -> CommResult<CanMessage> {
        let frame = self.socket.read_frame()
            .map_err(|e| CommError::SocketError(e.to_string()))?;
        
        Ok(CanMessage {
            id: frame.id(),
            data: frame.data().to_vec(),
            rtr: false,
            extended: false,
        })
    }
}

// Mock CAN 设备实现
struct MockCanSocket {
    tx: mpsc::Sender<CanMessage>,
    rx: Arc<Mutex<mpsc::Receiver<CanMessage>>>,
}

#[async_trait::async_trait]
impl CanSocket for MockCanSocket {
    async fn write_frame(&self, frame: &CanMessage) -> CommResult<()> {
        self.tx.send(frame.clone()).await
            .map_err(|e| CommError::SocketError(e.to_string()))
    }

    async fn read_frame(&self) -> CommResult<CanMessage> {
        let mut rx = self.rx.lock().await;
        rx.recv().await
            .ok_or_else(|| CommError::SocketError("No data available".to_string()))
    }
}

impl CanConnection {
    pub async fn new(interface: &str) -> CommResult<Self> {
        let socket: Box<dyn CanSocket + Send> = if interface.starts_with("mock") {
            // 创建 Mock Socket
            let (tx1, rx1) = mpsc::channel(100);
            let (tx2, rx2) = mpsc::channel(100);
            Box::new(MockCanSocket { 
                tx: tx1, 
                rx: Arc::new(Mutex::new(rx2))  // 包装 receiver
            })
        } else {
            // 创建真实 Socket
            let real_socket = socketcan::CANSocket::open(interface)
                .map_err(|e| CommError::SocketError(e.to_string()))?;
            real_socket.set_nonblocking(true)
                .map_err(|e| CommError::SocketError(e.to_string()))?;
            Box::new(RealCanSocket { socket: real_socket })
        };

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
            socket.write_frame(&message).await?;
        }
        Ok(())
    }

    fn get_message_priority(&self, message: &CanMessage) -> u8 {
        (message.id >> 24) as u8
    }

    pub async fn receive_message(&self) -> CommResult<CanMessage> {
        let socket = self.socket.lock().await;
        socket.read_frame().await
    }
}
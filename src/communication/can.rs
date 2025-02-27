use super::{buffer::MessageBuffer, filter::MessageFilter, priority::PriorityQueue, types::*, CommError, CommResult};
use std::sync::{Arc};
use tokio::sync::{mpsc, Mutex, MutexGuard};
use tracing::info;

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
            frame.extended,
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
        // 模拟写入延迟
        tokio::time::sleep(tokio::time::Duration::from_millis(10)).await;

        // 模拟写入成功率 95%
        if rand::random::<f32>() < 0.95 {
            match self.tx.send(frame.clone()).await {
                Ok(_) => {
                    Ok(())
                }
                Err(e) => {
                    Err(CommError::SocketError(format!("发送失败: {}", e)))
                }
            }
        } else {
            Err(CommError::SocketError("模拟写入失败".to_string()))
        }
    }

    async fn read_frame(&self) -> CommResult<CanMessage> {
        // 模拟读取延迟
        tokio::time::sleep(tokio::time::Duration::from_millis(5)).await;

        let mut rx = self.rx.lock().await;
        match rx.recv().await {
            Some(msg) => {
                Ok(msg)
            }
            None => {
                Err(CommError::SocketError("无数据可读".to_string()))
            }
        }
    }
}

impl CanConnection {
    pub async fn new(interface: &str) -> CommResult<Self> {
        let socket: Box<dyn CanSocket + Send> = if interface.starts_with("mock") {
            // 增大通道容量或使用无界通道
            let (tx, rx) = mpsc::channel(1000);

            // 创建消费者任务
            let rx_clone = Arc::new(Mutex::new(rx));
            let rx_for_task = rx_clone.clone();
            tokio::spawn(async move {
                loop {
                    let mut rx = rx_for_task.lock().await;
                    let _ = rx.recv().await;  // 持续消费消息
                }
            });

            Box::new(MockCanSocket {
                tx: tx.clone(),
                rx: rx_clone,
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
        {
            let mut queue = self.priority_queue.lock().await;
            let priority = self.get_message_priority(&message);
            queue.push(message, priority);
        }
        let result = self.process_queue().await;
        result
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
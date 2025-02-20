use std::collections::BinaryHeap;
use std::cmp::Ordering;
use super::types::CanMessage;

#[derive(Debug)]
struct PrioritizedMessage {
    message: CanMessage,
    priority: u8,
    timestamp: std::time::SystemTime,
}

impl PartialEq for PrioritizedMessage {
    fn eq(&self, other: &Self) -> bool {
        self.priority == other.priority
    }
}

impl Eq for PrioritizedMessage {}

impl PartialOrd for PrioritizedMessage {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

impl Ord for PrioritizedMessage {
    fn cmp(&self, other: &Self) -> Ordering {
        other.priority.cmp(&self.priority)
            .then_with(|| self.timestamp.cmp(&other.timestamp))
    }
}

#[derive(Debug)]
pub struct PriorityQueue {
    queue: BinaryHeap<PrioritizedMessage>,
}

impl PriorityQueue {
    pub fn new() -> Self {
        Self {
            queue: BinaryHeap::new(),
        }
    }

    pub fn push(&mut self, message: CanMessage, priority: u8) {
        self.queue.push(PrioritizedMessage {
            message,
            priority,
            timestamp: std::time::SystemTime::now(),
        });
    }

    pub fn pop(&mut self) -> Option<CanMessage> {
        self.queue.pop().map(|pm| pm.message)
    }
}
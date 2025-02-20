use super::types::*;
use std::collections::HashMap;
use std::time::{SystemTime, Duration};

pub struct QueueEstimator {
    approach_states: HashMap<u32, ApproachState>,
    vehicle_length: f32,  // 平均车长（米）
}

struct ApproachState {
    last_update: SystemTime,
    arrival_rate: f32,   // 车辆到达率（辆/秒）
    service_rate: f32,   // 服务率（辆/秒）
    current_queue: f32,  // 当前队长（米）
}

impl QueueEstimator {
    pub fn new() -> Self {
        Self {
            approach_states: HashMap::new(),
            vehicle_length: 5.0,
        }
    }

    pub fn update_state(&mut self, 
        approach_id: u32, 
        volume: f32,
        occupancy: f32,
        is_green: bool,
    ) {
        let now = SystemTime::now();
        let state = self.approach_states.entry(approach_id)
            .or_insert_with(|| ApproachState {
                last_update: now,
                arrival_rate: 0.0,
                service_rate: 0.0,
                current_queue: 0.0,
            });

        // 更新到达率和服务率
        state.arrival_rate = volume / 3600.0; // 转换为每秒
        state.service_rate = if is_green { 0.5 } else { 0.0 }; // 简化的服务率模型

        // 更新队列长度
        if let Ok(elapsed) = now.duration_since(state.last_update) {
            let dt = elapsed.as_secs_f32();
            let arrivals = state.arrival_rate * dt;
            let departures = state.service_rate * dt;
            
            state.current_queue += (arrivals - departures) * self.vehicle_length;
            state.current_queue = state.current_queue.max(0.0);
        }

        state.last_update = now;
    }

    pub fn estimate_queue(&self, approach_id: u32) -> Option<QueueEstimate> {
        self.approach_states.get(&approach_id).map(|state| {
            let vehicle_count = (state.current_queue / self.vehicle_length).ceil() as u32;
            let delay = Duration::from_secs_f32(
                state.current_queue / state.service_rate.max(0.1)
            );

            QueueEstimate {
                approach_id,
                queue_length: state.current_queue,
                vehicle_count,
                delay,
                timestamp: SystemTime::now(),
            }
        })
    }
}
use super::types::*;
use super::estimator::QueueEstimator;
use std::time::Duration;

pub struct OptimizationAlgorithm {
    min_cycle: Duration,
    max_cycle: Duration,
    queue_estimator: QueueEstimator,
}

impl OptimizationAlgorithm {
    pub fn new(min_cycle: Duration, max_cycle: Duration) -> Self {
        Self {
            min_cycle,
            max_cycle,
            queue_estimator: QueueEstimator::new(),
        }
    }

    pub fn optimize_cycle(&self, traffic_states: &[TrafficState]) -> CycleOptimization {
        // Webster公式计算最优周期
        let total_flow = traffic_states.iter().map(|s| s.volume).sum::<f32>();
        let saturation_flow = 1800.0; // 饱和流率（辆/小时）
        let lost_time = 4.0; // 损失时间（秒）
        
        let optimal_cycle = 1.5 * lost_time + 5.0 / (1.0 - total_flow / saturation_flow);
        let cycle_length = Duration::from_secs_f32(
            optimal_cycle.clamp(
                self.min_cycle.as_secs_f32(),
                self.max_cycle.as_secs_f32()
            )
        );

        // 计算相位分配
        let mut phases = Vec::new();
        let total_demand: f32 = traffic_states.iter()
            .map(|s| s.volume * s.occupancy)
            .sum();

        for (i, state) in traffic_states.iter().enumerate() {
            let demand = state.volume * state.occupancy;
            let split_ratio = demand / total_demand;
            
            phases.push(PhaseOptimization {
                phase_id: i as u32 + 1,
                split_ratio,
                min_green: Duration::from_secs(10),
                max_green: Duration::from_secs(60),
                target_green: Duration::from_secs_f32(
                    cycle_length.as_secs_f32() * split_ratio
                ),
            });
        }

        CycleOptimization {
            cycle_length,
            phases,
            performance_index: self.calculate_performance_index(traffic_states),
        }
    }

    fn calculate_performance_index(&self, states: &[TrafficState]) -> f32 {
        // 计算性能指标（延误最小化）
        let total_delay: f32 = states.iter()
            .map(|s| {
                let queue = s.queue_length;
                let speed = s.average_speed.max(1.0); // 避免除以零
                queue / speed // 简化的延误计算
            })
            .sum();
        
        -total_delay // 负值表示需要最小化
    }
}
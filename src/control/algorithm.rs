use super::types::*;
use super::error::ControlResult;
use std::time::Duration;

pub trait SignalControl: Send + Sync {
    fn calculate_splits(&self, detections: &[VehicleDetection]) -> ControlResult<Vec<Duration>>;
    fn optimize_timing(&self, current_plan: &TimingPlan) -> ControlResult<TimingPlan>;
    fn handle_coordination(&self, offset: Duration) -> ControlResult<TimingPlan>;
    fn get_current_status(&self) -> ControlResult<SignalStatus>;
}

pub struct AdaptiveController {
    signal_id: u32,
    current_plan: TimingPlan,
    current_status: SignalStatus,
    detection_buffer: Vec<VehicleDetection>,
    coordination_enabled: bool,
}

impl AdaptiveController {
    pub fn new(signal_id: u32, initial_plan: TimingPlan) -> Self {
        let status = SignalStatus {
            signal_id,
            phases: Vec::new(),
            current_plan: Some(initial_plan.clone()),
            mode: SignalMode::Normal,
        };

        Self {
            signal_id,
            current_plan: initial_plan,
            current_status: status,
            detection_buffer: Vec::new(),
            coordination_enabled: false,
        }
    }

    pub fn add_detection(&mut self, detection: VehicleDetection) {
        self.detection_buffer.push(detection);
    }

    fn calculate_phase_demand(&self, phase_id: u32) -> ControlResult<f32> {
        let phase_detections: Vec<&VehicleDetection> = self.detection_buffer.iter()
            .filter(|d| self.is_phase_detector(phase_id, d.detector_id))
            .collect();

        let volume = phase_detections.len() as f32;
        let avg_occupancy = phase_detections.iter()
            .filter(|d| d.presence)
            .count() as f32 / phase_detections.len() as f32;

        Ok(volume * 0.7 + avg_occupancy * 0.3)
    }

    fn update_timing_plan(&mut self) -> ControlResult<()> {
        let mut new_plan = self.current_plan.clone();
        
        // 计算各相位需求
        let mut phase_demands = Vec::new();
        for phase in &self.current_plan.phases {
            let demand = self.calculate_phase_demand(phase.phase_id)?;
            phase_demands.push((phase.phase_id, demand));
        }

        // 调整相位时长
        let total_demand: f32 = phase_demands.iter().map(|(_, d)| d).sum();
        for (phase_id, demand) in phase_demands {
            if let Some(phase) = new_plan.phases.iter_mut()
                .find(|p| p.phase_id == phase_id) {
                let split_ratio = demand / total_demand;
                let new_split = Duration::from_secs_f32(
                    new_plan.cycle_length.as_secs_f32() * split_ratio
                );
                phase.split = new_split;
            }
        }

        self.current_plan = new_plan;
        Ok(())
    }

    fn is_phase_detector(&self, phase_id: u32, detector_id: u32) -> bool {
        // TODO: 实现检测器与相位的对应关系
        // 这部分需要参考原C++版本的配置
        true
    }

    fn is_coordination_phase(&self, phase_id: u32) -> bool {
        // 判断是否为干线协调相位
        // TODO: 根据实际配置确定协调相位
        phase_id == 2 || phase_id == 6
    }
}

impl SignalControl for AdaptiveController {
    fn calculate_splits(&self, detections: &[VehicleDetection]) -> ControlResult<Vec<Duration>> {
        let mut splits = Vec::new();
        let cycle_time = self.current_plan.cycle_length.as_secs_f32();

        for phase in &self.current_plan.phases {
            let demand = self.calculate_phase_demand(phase.phase_id)?;
            let min_time = 0.15 * cycle_time; // 最小绿灯时间比例
            let max_time = 0.45 * cycle_time; // 最大绿灯时间比例
            
            let split_time = (min_time + demand * (max_time - min_time))
                .clamp(min_time, max_time);
            
            splits.push(Duration::from_secs_f32(split_time));
        }

        Ok(splits)
    }

    fn optimize_timing(&self, current_plan: &TimingPlan) -> ControlResult<TimingPlan> {
        let mut optimized_plan = current_plan.clone();
        
        // 基于历史数据优化周期时长
        let avg_demand = self.detection_buffer.iter()
            .filter(|d| d.presence)
            .count() as f32 / self.detection_buffer.len() as f32;

        let new_cycle = if avg_demand > 0.8 {
            current_plan.cycle_length + Duration::from_secs(5)
        } else if avg_demand < 0.3 {
            current_plan.cycle_length - Duration::from_secs(5)
        } else {
            current_plan.cycle_length
        };

        optimized_plan.cycle_length = new_cycle;
        Ok(optimized_plan)
    }

    fn handle_coordination(&self, offset: Duration) -> ControlResult<TimingPlan> {
        if !self.coordination_enabled {
            return Ok(self.current_plan.clone());
        }

        let mut coordinated_plan = self.current_plan.clone();
        coordinated_plan.offset = offset;

        // 调整相位序列以适应协调控制
        coordinated_plan.phases.sort_by_key(|p| p.phase_sequence);
        
        // 确保关键相位的绿灯时长满足协调要求
        for phase in &mut coordinated_plan.phases {
            if self.is_coordination_phase(phase.phase_id) {
                let min_split = Duration::from_secs(15); // 协调相位最小绿灯时间
                if phase.split < min_split {
                    phase.split = min_split;
                }
            }
        }

        Ok(coordinated_plan)
    }

    fn get_current_status(&self) -> ControlResult<SignalStatus> {
        let mut status = self.current_status.clone();
        
        // 更新各相位剩余时间
        for phase in &mut status.phases {
            if let Some(config) = self.current_plan.phases.iter()
                .find(|p| p.phase_id == phase.phase_id) {
                match phase.state {
                    PhaseState::Green => {
                        phase.remaining_time = config.split;
                    }
                    PhaseState::Yellow => {
                        phase.remaining_time = Duration::from_secs(3); // 标准黄灯时间
                    }
                    PhaseState::Red => {
                        phase.remaining_time = Duration::from_secs(2); // 标准全红时间
                    }
                    PhaseState::Flash => {
                        phase.remaining_time = Duration::from_secs(0);
                    }
                }
            }
        }

        Ok(status)
    }
}

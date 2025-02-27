use std::collections::HashMap;
use std::io;
use std::io::Write;
use std::ops::Deref;
use super::{error::{ControlError, ControlResult}, types::*};
use crate::hardware::HardwareManager;
use tokio::sync::Mutex;
use std::sync::Arc;
use std::time::Duration;
use tracing::{info, error, debug};

pub struct ControlManager {
    hardware: Arc<HardwareManager>,
    state: Arc<Mutex<ControllerState>>,
    timing_plans: Arc<Mutex<HashMap<u32, TimingPlan>>>,
    phases: Arc<Mutex<HashMap<u32, Phase>>>,
}

impl ControlManager {
    pub fn new(hardware: Arc<HardwareManager>) -> Self {
        let state = ControllerState {
            current_plan: 0,
            current_phase: 0,
            phase_elapsed_time: 0,
            cycle_elapsed_time: 0,
            mode: ControlMode::AllRed,
        };

        Self {
            hardware,
            state: Arc::new(Mutex::new(state)),
            timing_plans: Arc::new(Mutex::new(HashMap::new())),
            phases: Arc::new(Mutex::new(HashMap::new())),
        }
    }

    pub async fn initialize(&self) -> ControlResult<()> {
        self.load_configuration().await?;
        self.set_mode(ControlMode::AllRed).await?;
        Ok(())
    }

    pub async fn set_mode(&self, mode: ControlMode) -> ControlResult<()> {
        {
            let mut state = self.state.lock().await;
            state.mode = mode;
        }
        self.update_outputs().await?;
        Ok(())
    }

    pub async fn update_timing_plan(&self, plan: TimingPlan) -> ControlResult<()> {
        let mut plans = self.timing_plans.lock().await;
        self.validate_timing_plan(&plan)?;
        plans.insert(plan.id, plan.clone());
        info!("更新配时方案: {}", plan.id);
        Ok(())
    }

    pub async fn run_cycle(&self) -> ControlResult<()> {
        info!("run_cycle ...");
        let mode = {
            let state = self.state.lock().await;
            state.mode
        };
        info!("循环：{:?}",mode);

        match mode {
            ControlMode::Fixed => self.run_fixed_time_cycle().await,
            ControlMode::Vehicle => self.run_vehicle_responsive_cycle().await,
            ControlMode::Flash => self.run_flash_mode().await,
            ControlMode::AllRed => self.run_all_red_mode().await,
            ControlMode::Manual => Ok(()),
            ControlMode::Error => Err(ControlError::StateError("系统处于错误状态".to_string())),
        }
    }

    async fn load_configuration(&self) -> ControlResult<()> {
        // 从配置文件加载相位和配时方案
        let mut phases = self.phases.lock().await;
        let mut plans = self.timing_plans.lock().await;

        info!("从配置文件加载相位和配时方案");

        // TODO: 从配置文件加载
        // 临时添加测试数据
        let phase1 = Phase {
            id: 1,
            name: "东西直行".to_string(),
            movements: vec![],
            min_green: Duration::from_secs(10),
            max_green: Duration::from_secs(60),
            yellow_time: Duration::from_secs(3),
            red_clearance: Duration::from_secs(2),
        };
        phases.insert(1, phase1);

        let plan = TimingPlan {
            id: 1,
            name: "默认配时方案".to_string(),
            cycle_length: Duration::from_secs(120),
            offset: Duration::from_secs(0),
            phases: vec![
                PhaseConfig {
                    phase_id: 1,
                    split: Duration::from_secs(30),
                    phase_sequence: 1,
                },
            ],
        };
        plans.insert(1, plan);

        // 设置当前配时方案
        {
            let mut state = self.state.lock().await;
            state.current_plan = 1;  // 设置为已加载的配时方案ID
        }

        Ok(())
    }

    async fn update_outputs(&self) -> ControlResult<()> {
        info!("输出到硬件");
        let state_data = {
            let state = self.state.lock().await;
            (state.mode, state.current_plan, state.phase_elapsed_time)
        };
        let phases_data = {
            let phases = self.phases.lock().await;
            phases.iter().map(|(id, phase)| (*id, phase.clone())).collect::<Vec<_>>()
        };
        let plans_data = {
            let plans = self.timing_plans.lock().await;
            plans.get(&state_data.1).cloned()
        };

        for (phase_id, phase) in phases_data {  // Remove .iter()
            let status = if let Some(plan) = &plans_data {
                if let Some(phase_config) = plan.phases.iter().find(|p| p.phase_id == phase_id) {  // phase_id is now u32
                    let elapsed = state_data.2 as u64;

                    if elapsed < phase_config.split.as_secs() {
                        PhaseState::Green
                    } else if elapsed < phase_config.split.as_secs() + phase.yellow_time.as_secs() {
                        PhaseState::Yellow
                    } else {
                        PhaseState::Red
                    }
                } else {
                    PhaseState::Red
                }
            } else {
                PhaseState::Red
            };

            if let Err(e) = self.hardware.update_phase_output(phase_id, status).await {
                return Err(ControlError::HardwareError(e.to_string()));
            }
        }

        Ok(())
    }

    async fn run_fixed_time_cycle(&self) -> ControlResult<()> {
        info!("定周期控制1...");

        // 获取并更新状态
        let (current_plan, need_switch) = {
            let mut state = self.state.lock().await;
            let plans = self.timing_plans.lock().await;
            let mut need_switch = false;
            info!("定周期控制2,state:{:?}",state);
            if let Some(plan) = plans.get(&state.current_plan) {
                state.cycle_elapsed_time += 1;
                state.phase_elapsed_time += 1;
                info!("定周期控制3,plan:{:?}",plan);
                if let Some(current_phase) = plan.phases.iter()
                    .find(|p| p.phase_id == state.current_phase) {
                    info!("定周期控制4,phase:{:?}",current_phase);
                    if state.phase_elapsed_time as u64 >= current_phase.split.as_secs() {
                        need_switch = true;
                        info!("定周期控制5,need_switch:{:?}",need_switch);
                    }
                }

                if state.cycle_elapsed_time as u64 >= plan.cycle_length.as_secs() {
                    state.cycle_elapsed_time = 0;
                }
            }
            (state.current_plan, need_switch)
        };  // 这里释放所有锁

        // 如果需要切换相位，单独处理
        if need_switch {
            info!("定周期控制6,to need_switch");
            let mut state = self.state.lock().await;
            let plans = self.timing_plans.lock().await;
            if let Some(plan) = plans.get(&current_plan) {
                info!("定周期控制7,switch_to_next_phase");
                self.switch_to_next_phase(&mut state, plan).await?;
            }
        }

        info!("定周期控制8,update_outputs");
        self.update_outputs().await?;
        info!("定周期控制9,完成");
        Ok(())
    }

    async fn switch_to_next_phase(&self, state: &mut ControllerState, plan: &TimingPlan) -> ControlResult<()> {
        let current_idx = plan.phases.iter()
            .position(|p| p.phase_id == state.current_phase)
            .unwrap_or(0);

        let next_idx = (current_idx + 1) % plan.phases.len();
        state.current_phase = plan.phases[next_idx].phase_id;
        state.phase_elapsed_time = 0;

        info!("切换到相位: {}", state.current_phase);
        Ok(())
    }

    async fn get_phase_status(&self, phase_id: u32, state: &ControllerState, phases: &HashMap<u32, Phase>) -> ControlResult<PhaseState> {
        let plans = self.timing_plans.lock().await;

        if let (Some(phase), Some(plan)) = (phases.get(&phase_id), plans.get(&state.current_plan)) {
            if let Some(phase_config) = plan.phases.iter().find(|p| p.phase_id == phase_id) {
                let elapsed = state.phase_elapsed_time as u64;

                if elapsed < phase_config.split.as_secs() {
                    Ok(PhaseState::Green)
                } else if elapsed < phase_config.split.as_secs() + phase.yellow_time.as_secs() {
                    Ok(PhaseState::Yellow)
                } else {
                    Ok(PhaseState::Red)
                }
            } else {
                Ok(PhaseState::Red)
            }
        } else {
            Err(ControlError::StateError("相位未找到".to_string()))
        }
    }

    fn validate_timing_plan(&self, plan: &TimingPlan) -> ControlResult<()> {
        // 验证周期时长
        if plan.cycle_length.as_secs() < 30 || plan.cycle_length.as_secs() > 180 {
            return Err(ControlError::ValidationError(
                "周期时长必须在30-180秒之间".to_string()
            ));
        }

        // 验证相位配置
        let total_split: Duration = plan.phases.iter()
            .map(|p| p.split)
            .sum();
        if total_split != plan.cycle_length {
            return Err(ControlError::ValidationError(
                "相位分配总和必须等于周期时长".to_string()
            ));
        }

        Ok(())
    }

    async fn run_vehicle_responsive_cycle(&self) -> ControlResult<()> {
        let mut state = self.state.lock().await;
        let plans = self.timing_plans.lock().await;

        if let Some(plan) = plans.get(&state.current_plan) {
            state.cycle_elapsed_time += 1;
            state.phase_elapsed_time += 1;

            // 获取当前相位配置
            if let Some(current_phase) = plan.phases.iter()
                .find(|p| p.phase_id == state.current_phase) {

                // 检查是否需要切换相位
                let should_switch = if let Some(detection) = self.check_vehicle_presence(current_phase.phase_id).await? {
                    // 有车辆时延长绿灯，但不超过最大绿灯时间
                    let phases = self.phases.lock().await;
                    if let Some(phase) = phases.get(&current_phase.phase_id) {
                        state.phase_elapsed_time as u64 >= phase.max_green.as_secs()
                    } else {
                        true
                    }
                } else {
                    // 无车辆时使用最小绿灯时间
                    let phases = self.phases.lock().await;
                    if let Some(phase) = phases.get(&current_phase.phase_id) {
                        state.phase_elapsed_time as u64 >= phase.min_green.as_secs()
                    } else {
                        true
                    }
                };

                if should_switch {
                    self.switch_to_next_phase(&mut state, plan).await?;
                }
            }
        }

        self.update_outputs().await?;
        Ok(())
    }

    async fn run_flash_mode(&self) -> ControlResult<()> {
        let state = self.state.lock().await;
        let phases = self.phases.lock().await;

        for (phase_id, _) in phases.iter() {
            // 在闪光模式下，所有相位都闪烁
            self.hardware.update_phase_output(*phase_id, PhaseState::Flash).await
                .map_err(|e| ControlError::HardwareError(e.to_string()))?;
        }

        Ok(())
    }

    async fn run_all_red_mode(&self) -> ControlResult<()> {
        let state = self.state.lock().await;
        let phases = self.phases.lock().await;

        for (phase_id, _) in phases.iter() {
            // 在全红模式下，所有相位都为红灯
            self.hardware.update_phase_output(*phase_id, PhaseState::Red).await
                .map_err(|e| ControlError::HardwareError(e.to_string()))?;
        }

        Ok(())
    }

    async fn check_vehicle_presence(&self, phase_id: u32) -> ControlResult<Option<bool>> {
        // TODO: 实现车辆检测逻辑
        Ok(None)
    }

    pub async fn update_coordination_offset(&self, offset: Duration) -> ControlResult<()> {
        let mut state = self.state.lock().await;
        let mut plans = self.timing_plans.lock().await;

        if let Some(plan) = plans.get_mut(&state.current_plan) {
            plan.offset = offset;
            info!("更新协调偏移量: {:?}", offset);
            self.update_outputs().await?;
        } else {
            return Err(ControlError::StateError("当前无活动配时方案".to_string()));
        }

        Ok(())
    }

    pub async fn disable_coordination(&self) -> ControlResult<()> {
        let mut state = self.state.lock().await;
        let mut plans = self.timing_plans.lock().await;

        if let Some(plan) = plans.get_mut(&state.current_plan) {
            plan.offset = Duration::from_secs(0);
            info!("已禁用协调控制");
            self.update_outputs().await?;
        }

        Ok(())
    }

    pub async fn is_coordinated(&self) -> bool {
        let state = self.state.lock().await;
        let plans = self.timing_plans.lock().await;

        if let Some(plan) = plans.get(&state.current_plan) {
            plan.offset.as_secs() > 0
        } else {
            false
        }
    }

    pub async fn get_current_phase(&self) -> u32 {
        let state = self.state.lock().await;
        state.current_phase
    }

    pub async fn get_phase_elapsed_time(&self) -> u32 {
        let state = self.state.lock().await;
        state.phase_elapsed_time
    }

    pub async fn get_current_mode(&self) -> ControlMode {
        let state = self.state.lock().await;
        state.mode
    }

    pub async fn get_status(&self) -> ControllerState {
        self.state.lock().await.clone()
    }
}
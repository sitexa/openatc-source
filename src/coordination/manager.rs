use super::algorithm::GreenWaveCalculator;
use super::types::*;
use crate::control::ControlManager;
use std::collections::HashMap;
use std::sync::Arc;
use tokio::sync::Mutex;
use tracing::{error, info, warn};

pub struct CoordinationManager {
    intersections: Arc<Mutex<HashMap<u32, IntersectionInfo>>>,
    active_plan: Arc<Mutex<Option<CoordinationPlan>>>,
    control_managers: HashMap<u32, Arc<ControlManager>>,
    calculator: GreenWaveCalculator,
}

impl CoordinationManager {
    pub fn new() -> Self {
        Self {
            intersections: Arc::new(Mutex::new(HashMap::new())),
            active_plan: Arc::new(Mutex::new(None)),
            control_managers: HashMap::new(),
            calculator: GreenWaveCalculator::new(),
        }
    }

    pub async fn add_intersection(&mut self, info: IntersectionInfo, control: Arc<ControlManager>) {
        let mut intersections = self.intersections.lock().await;
        let id = info.id;  // Store id before moving info
        intersections.insert(id, info);
        self.control_managers.insert(id, control);
    }

    pub async fn activate_plan(&self, plan: CoordinationPlan) {
        let mut active = self.active_plan.lock().await;
        *active = Some(plan.clone());

        // 计算绿波带
        let green_wave = self.calculator.calculate_green_wave(&plan);
        
        // 更新各路口的偏移量
        for intersection in &plan.intersections {
            if let Some(control) = self.control_managers.get(&intersection.intersection_id) {
                control.update_coordination_offset(intersection.offset).await
                    .unwrap_or_else(|e| {
                        error!("更新路口{}偏移量失败: {}", intersection.intersection_id, e);
                    });
            }
        }

        info!("协调控制方案已激活: {}", plan.name);
    }

    pub async fn deactivate_plan(&self) {
        let mut active = self.active_plan.lock().await;
        if let Some(plan) = active.take() {
            // 恢复各路口独立运行
            for intersection in &plan.intersections {
                if let Some(control) = self.control_managers.get(&intersection.intersection_id) {
                    control.disable_coordination().await
                        .unwrap_or_else(|e| {
                            error!("禁用路口{}协调失败: {}", intersection.intersection_id, e);
                        });
                }
            }
            info!("协调控制方案已停用");
        }
    }

    pub async fn adjust_offsets(&self) {
        if let Some(plan) = &*self.active_plan.lock().await {
            // 根据实时交通状况调整偏移量
            let adjusted_offsets = self.calculator.optimize_offsets(plan);
            
            // 更新各路口的偏移量
            for offset in adjusted_offsets {
                if let Some(control) = self.control_managers.get(&offset.intersection_id) {
                    control.update_coordination_offset(offset.offset).await
                        .unwrap_or_else(|e| {
                            error!("调整路口{}偏移量失败: {}", offset.intersection_id, e);
                        });
                }
            }
        }
    }
}
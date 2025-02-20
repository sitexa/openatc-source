use super::types::*;
use crate::control::ControlManager;
use axum::{
    routing::{get, post},
    Router, Json,
    extract::State,
};
use std::sync::Arc;
use std::time::SystemTime;
use tracing::{info, error};

pub struct WebServer {
    control: Arc<ControlManager>,
    port: u16,
}

impl WebServer {
    pub fn new(control: Arc<ControlManager>, port: u16) -> Self {
        Self { control, port }
    }

    pub async fn start(&self) {
        let app = Router::new()
            .route("/api/status", get(Self::get_status))
            .route("/api/command", post(Self::handle_command))
            .with_state(self.control.clone());

        let addr = format!("0.0.0.0:{}", self.port);
        info!("Web服务器启动在 {}", addr);

        axum::Server::bind(&addr.parse().unwrap())
            .serve(app.into_make_service())
            .await
            .unwrap();
    }

    async fn get_status(
        State(control): State<Arc<ControlManager>>,
    ) -> Json<ApiResponse<SignalStatus>> {
        let status = SignalStatus {
            intersection_id: 1,  // TODO: 从控制器获取
            current_phase: control.get_current_phase().await,
            phase_elapsed_time: control.get_phase_elapsed_time().await,
            mode: control.get_current_mode().await.to_string(),
            coordination_active: control.is_coordinated().await,
        };

        Json(ApiResponse {
            success: true,
            message: "获取状态成功".to_string(),
            data: Some(status),
            timestamp: SystemTime::now(),
        })
    }

    async fn handle_command(
        State(control): State<Arc<ControlManager>>,
        Json(command): Json<RemoteCommand>,
    ) -> Json<ApiResponse<()>> {
        info!("收到远程命令: {:?}", command);

        match command.command_type {
            CommandType::ChangeMode => {
                // 处理模式切换命令
            }
            CommandType::UpdatePlan => {
                // 处理更新配时方案命令
            }
            CommandType::ForcePhase => {
                // 处理强制相位命令
            }
            CommandType::EnableCoordination => {
                // 处理启用协调命令
            }
            CommandType::DisableCoordination => {
                // 处理禁用协调命令
            }
            CommandType::Heartbeat => {
                // 处理心跳包
                info!("收到心跳包");
            }
        }

        Json(ApiResponse {
            success: true,
            message: "命令执行成功".to_string(),
            data: None,
            timestamp: SystemTime::now(),
        })
    }
}
use traeatc::fault::{FaultManager, FaultRecord, FaultType, FaultSeverity, FaultStatus};
use std::time::SystemTime;

#[tokio::test]
async fn test_fault_reporting() {
    let manager = FaultManager::new();

    let fault = FaultRecord {
        id: 1,
        fault_type: FaultType::Hardware,
        severity: FaultSeverity::Major,
        timestamp: SystemTime::now(),
        description: "测试故障".to_string(),
        source: "测试模块".to_string(),
        status: FaultStatus::Active,
        recovery_action: None,
    };

    assert!(manager.report_fault(fault).await.is_ok());

    let active_faults = manager.get_active_faults().await;
    assert_eq!(active_faults.len(), 1);
}
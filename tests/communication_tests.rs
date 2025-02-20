use traeatc::communication::{CanConnection, CanMessage};

#[tokio::test]
async fn test_can_connection() {
    let connection = CanConnection::new("vcan0").await;
    assert!(connection.is_ok());
}

#[tokio::test]
async fn test_can_message() {
    let message = CanMessage {
        id: 0x123,
        data: vec![0x01, 0x02, 0x03, 0x04],
        extended: false,
        rtr: false,
    };

    assert_eq!(message.id, 0x123);
    assert_eq!(message.data.len(), 4);
}
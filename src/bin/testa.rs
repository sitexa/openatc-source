use tokio::time::{interval, Duration};

async fn task_1() {
    println!("任务 1 执行");
}

async fn task_2() {
    println!("任务 2 执行");
}

async fn task_3() {
    println!("任务 3 执行");
}

#[tokio::main]
async fn main() {
    // 创建定时器，每 5 秒触发一次
    let mut interval = interval(Duration::from_secs(5));

    loop {
        // 等待定时器触发
        interval.tick().await;

        // 在每次定时器触发时，异步地启动所有任务
        tokio::spawn(task_1());
        tokio::spawn(task_2());
        tokio::spawn(task_3());
    }
}

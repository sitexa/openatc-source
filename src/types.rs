pub enum ParameterValue {
    Integer(i64),
    Float(f64),
    String(String),
    Boolean(bool),
}

pub struct Parameter {
    pub id: u32,
    pub name: String,
    pub value: ParameterValue,
    pub unit: Option<String>,
}

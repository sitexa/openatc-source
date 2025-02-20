use super::types::CanMessage;

#[derive(Debug, Clone)]
pub struct CanFilter {
    pub id_mask: u32,
    pub id_match: u32,
    pub extended: bool,
}

#[derive(Debug)]
pub struct MessageFilter {
    filters: Vec<CanFilter>,
}

impl MessageFilter {
    pub fn new() -> Self {
        Self {
            filters: Vec::new(),
        }
    }

    pub fn add_filter(&mut self, filter: CanFilter) {
        self.filters.push(filter);
    }

    pub fn matches(&self, message: &CanMessage) -> bool {
        if self.filters.is_empty() {
            return true;
        }

        self.filters.iter().any(|filter| {
            (message.id & filter.id_mask) == (filter.id_match & filter.id_mask)
                && message.extended == filter.extended
        })
    }
}
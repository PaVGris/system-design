#include <storage/items_storage.hpp>

namespace item_service {

int ItemsStorage::AddItem(const std::string& name,
                          const std::string& description,
                          double price,
                          int quantity) {
    int id = next_id_++;
    items_[id] = Item{id, name, description, price, quantity};
    return id;
}

std::optional<Item> ItemsStorage::GetItem(int id) const {
    auto it = items_.find(id);
    if (it == items_.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::vector<Item> ItemsStorage::GetAllItems() const {
    std::vector<Item> result;
    result.reserve(items_.size());
    for (const auto& [id, item] : items_) {
        result.push_back(item);
    }
    return result;
}

bool ItemsStorage::UpdateItem(int id,
                              const std::optional<std::string>& name,
                              const std::optional<std::string>& description,
                              const std::optional<double>& price,
                              const std::optional<int>& quantity) {
    auto it = items_.find(id);
    if (it == items_.end()) {
        return false;
    }

    if (name) it->second.name = *name;
    if (description) it->second.description = *description;
    if (price) it->second.price = *price;
    if (quantity) it->second.quantity = *quantity;

    return true;
}

bool ItemsStorage::DeleteItem(int id) {
    return items_.erase(id) > 0;
}

}  // namespace item_service
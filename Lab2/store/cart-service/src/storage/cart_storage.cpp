#include <storage/cart_storage.hpp>

namespace cart_service {

void CartStorage::AddItem(int user_id, int item_id, int quantity) {
    auto& cart = carts_[user_id];
    for (auto& item : cart) {
        if (item.item_id == item_id) {
            item.quantity = item.quantity.value_or(0) + quantity;
            return;
        }
    }
    cart.push_back({item_id, quantity});
}

std::vector<CartItem> CartStorage::GetCart(int user_id) const {
    auto it = carts_.find(user_id);
    if (it == carts_.end()) {
        return {};
    }
    return it->second;
}

bool CartStorage::RemoveItem(int user_id, int item_id) {
    auto it = carts_.find(user_id);
    if (it == carts_.end()) {
        return false;
    }

    auto& cart = it->second;
    for (auto iter = cart.begin(); iter != cart.end(); ++iter) {
        if (iter->item_id == item_id) {
            cart.erase(iter);
            return true;
        }
    }
    return false;
}

void CartStorage::ClearCart(int user_id) {
    carts_.erase(user_id);
}

}  // namespace cart_service
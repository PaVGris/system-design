#pragma once
#include <unordered_map>
#include <vector>
#include <userver/components/component_base.hpp>
#include <schemas/cart.hpp>

namespace cart_service {

class CartStorage final : public userver::components::ComponentBase {
public:
    static constexpr std::string_view kName = "cart-storage";

    CartStorage(const userver::components::ComponentConfig& config,
                const userver::components::ComponentContext& context)
        : ComponentBase(config, context) {}

    void AddItem(int user_id, int item_id, int quantity);

    std::vector<CartItem> GetCart(int user_id) const;

    bool RemoveItem(int user_id, int item_id);

    void ClearCart(int user_id);

private:
    std::unordered_map<int, std::vector<CartItem>> carts_;
};

}  // namespace cart_service
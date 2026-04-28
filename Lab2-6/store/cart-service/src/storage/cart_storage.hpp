#pragma once
#include <string>
#include <vector>
#include <userver/components/component_base.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/storages/mongo/pool.hpp>
#include <schemas/cart.hpp>

namespace cart_service {

class CartStorage final : public userver::components::ComponentBase {
public:
    static constexpr std::string_view kName = "cart-storage";

    CartStorage(const userver::components::ComponentConfig& config,
                const userver::components::ComponentContext& context);

    void AddItem(int user_id, const std::string& item_id, int quantity);
    std::vector<CartItem> GetItems(int user_id);
    void RemoveItem(int user_id, const std::string& item_id);
    void ClearCart(int user_id);

private:
    userver::storages::mongo::PoolPtr pool_;
};

} // namespace cart_service
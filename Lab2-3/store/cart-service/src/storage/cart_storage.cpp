#include "cart_storage.hpp"

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/component.hpp>

namespace cart_service {

CartStorage::CartStorage(const userver::components::ComponentConfig& config,
                         const userver::components::ComponentContext& context)
    : ComponentBase(config, context),
      pg_cluster_(context.FindComponent<userver::components::Postgres>("postgres-db").GetCluster()) {}

void CartStorage::AddItem(int user_id, int item_id, int quantity) {
    pg_cluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster,
                         "INSERT INTO cart_items (user_id, item_id, quantity) "
                         "VALUES ($1, $2, $3) "
                         "ON CONFLICT (user_id, item_id) "
                         "DO UPDATE SET quantity = cart_items.quantity + EXCLUDED.quantity",
                         user_id, item_id, quantity);
}

std::vector<CartItem> CartStorage::GetItems(int user_id) {
    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kSlave,
        "SELECT item_id, quantity FROM cart_items WHERE user_id = $1", 
        user_id);
    
    return result.AsContainer<std::vector<CartItem>>(userver::storages::postgres::kRowTag);
}

void CartStorage::RemoveItem(int user_id, int item_id) {
    pg_cluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster,
                         "DELETE FROM cart_items WHERE user_id = $1 AND item_id = $2",
                         user_id, item_id);
}

void CartStorage::ClearCart(int user_id) {
    pg_cluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster,
                         "DELETE FROM cart_items WHERE user_id = $1",
                         user_id);
}

} // namespace cart_service
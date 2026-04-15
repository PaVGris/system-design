#include <storage/items_storage.hpp>
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/cluster.hpp>

namespace item_service {

ItemsStorage::ItemsStorage(const userver::components::ComponentConfig& config,
                           const userver::components::ComponentContext& context)
    : ComponentBase(config, context),
      pg_cluster_(context.FindComponent<userver::components::Postgres>("postgres-db")
                  .GetCluster()) {}

int ItemsStorage::AddItem(const std::string& name,
                          const std::string& description,
                          double price,
                          int quantity) {
    // Используем RETURNING id и AsSingleRow для получения созданного ID
    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        "INSERT INTO items (name, description, price, quantity) "
        "VALUES ($1, $2, $3, $4) RETURNING id",
        name, description, price, quantity
    );
    
    return result.AsSingleRow<int>();
}

std::optional<Item> ItemsStorage::GetItem(int id) const {
    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kSlave,
        "SELECT id, name, description, price, quantity FROM items WHERE id = $1",
        id
    );
    
    if (result.IsEmpty()) {
        return std::nullopt;
    }
    
    // Автоматический маппинг в структуру Item (сгенерированную Chaotic)
    return result.AsSingleRow<Item>(userver::storages::postgres::kRowTag);
}

std::vector<Item> ItemsStorage::GetAllItems() const {
    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kSlave,
        "SELECT id, name, description, price, quantity FROM items ORDER BY id"
    );
    
    // Автоматическое наполнение вектора структур
    return result.AsContainer<std::vector<Item>>(userver::storages::postgres::kRowTag);
}

bool ItemsStorage::UpdateItem(int id,
                              const std::optional<std::string>& name,
                              const std::optional<std::string>& description,
                              const std::optional<double>& price,
                              const std::optional<int>& quantity) {
    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        R"(
        UPDATE items 
        SET 
            name = COALESCE($2, name),
            description = COALESCE($3, description),
            price = COALESCE($4, price),
            quantity = COALESCE($5, quantity)
        WHERE id = $1
        )",
        id, name, description, price, quantity
    );

    return result.RowsAffected() > 0;
}

bool ItemsStorage::DeleteItem(int id) {
    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        "DELETE FROM items WHERE id = $1",
        id
    );
    
    return result.RowsAffected() > 0;
}

}  // namespace item_service
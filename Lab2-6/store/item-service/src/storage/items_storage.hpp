#pragma once
#include <string>

#include <optional>
#include <vector>
#include <userver/components/component_base.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/storages/mongo/pool.hpp>

namespace item_service {

struct Item {
    std::string id;
    std::string name;
    std::string description;
    double price;
    int quantity;
};

class ItemsStorage final : public userver::components::ComponentBase {
public:
    static constexpr std::string_view kName = "items-storage";

    ItemsStorage(const userver::components::ComponentConfig& config,
                 const userver::components::ComponentContext& context);

    std::string AddItem(const std::string& name,
                const std::string& description,
                double price,
                int quantity);

    std::optional<Item> GetItem(const std::string& id) const;

    std::vector<Item> GetAllItems() const;

    bool UpdateItem(const std::string& id,
                    const std::optional<std::string>& name,
                    const std::optional<std::string>& description,
                    const std::optional<double>& price,
                    const std::optional<int>& quantity);

    bool DeleteItem(const std::string& id);

private:
    userver::storages::mongo::PoolPtr pool_;
};

}  // namespace item_service
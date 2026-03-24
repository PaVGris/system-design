#pragma once
#include <string>
#include <unordered_map>
#include <optional>
#include <vector>
#include <userver/components/component_base.hpp>

namespace item_service {

struct Item {
    int id;
    std::string name;
    std::string description;
    double price;
    int quantity;
};

class ItemsStorage final : public userver::components::ComponentBase {
public:
    static constexpr std::string_view kName = "items-storage";

    ItemsStorage(const userver::components::ComponentConfig& config,
                 const userver::components::ComponentContext& context)
        : ComponentBase(config, context) {}

    int AddItem(const std::string& name,
                const std::string& description,
                double price,
                int quantity);

    std::optional<Item> GetItem(int id) const;

    std::vector<Item> GetAllItems() const;

    bool UpdateItem(int id,
                    const std::optional<std::string>& name,
                    const std::optional<std::string>& description,
                    const std::optional<double>& price,
                    const std::optional<int>& quantity);

    bool DeleteItem(int id);

private:
    std::unordered_map<int, Item> items_;
    int next_id_ = 1;
};

}  // namespace item_service
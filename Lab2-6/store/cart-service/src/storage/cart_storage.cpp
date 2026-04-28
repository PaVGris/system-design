#include "cart_storage.hpp"
#include <userver/components/component_context.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/formats/bson/inline.hpp>
#include <userver/formats/bson/value_builder.hpp>

namespace cart_service {

using userver::formats::bson::MakeDoc;

CartStorage::CartStorage(const userver::components::ComponentConfig& config,
                         const userver::components::ComponentContext& context)
    : ComponentBase(config, context),
      pool_(context.FindComponent<userver::components::Mongo>("mongo-cart").GetPool()) {}

void CartStorage::AddItem(int user_id, const std::string& item_id, int quantity) {
    auto col = pool_->GetCollection("carts");
    auto result = col.UpdateOne(
        MakeDoc("user_id", user_id, "items.item_id", item_id),
        MakeDoc("$inc", MakeDoc("items.$.quantity", quantity))
    );
    if (result.MatchedCount() == 0) {
        col.UpdateOne(
            MakeDoc("user_id", user_id),
            MakeDoc("$push", MakeDoc("items", MakeDoc("item_id", item_id, "quantity", quantity))),
            userver::storages::mongo::options::Upsert{}
        );
    }
}

std::vector<CartItem> CartStorage::GetItems(int user_id) {
    auto col = pool_->GetCollection("carts");
    auto result = col.FindOne(MakeDoc("user_id", user_id));
    std::vector<CartItem> items;
    if (!result) return items;

    const auto& items_arr = (*result)["items"];
    if (!items_arr.IsArray()) return items;
    for (const auto& elem : items_arr) {
        CartItem ci;
        ci.item_id = elem["item_id"].As<std::string>();
        ci.quantity = elem["quantity"].As<int>();
        items.push_back(ci);
    }
    return items;
}

void CartStorage::RemoveItem(int user_id, const std::string& item_id) {
    auto col = pool_->GetCollection("carts");
    col.UpdateOne(
        MakeDoc("user_id", user_id),
        MakeDoc("$pull", MakeDoc("items", MakeDoc("item_id", item_id)))
    );
}

void CartStorage::ClearCart(int user_id) {
    auto col = pool_->GetCollection("carts");
    col.UpdateOne(
        MakeDoc("user_id", user_id),
        MakeDoc("$set", MakeDoc("items", userver::formats::bson::MakeArray()))
    );
}

} // namespace cart_service
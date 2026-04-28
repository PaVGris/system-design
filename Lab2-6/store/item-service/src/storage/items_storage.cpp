#include "items_storage.hpp"
#include <userver/components/component_context.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/formats/bson/inline.hpp>
#include <userver/formats/bson/value_builder.hpp>

namespace item_service {

using userver::formats::bson::MakeDoc;
using userver::formats::bson::MakeArray;

ItemsStorage::ItemsStorage(const userver::components::ComponentConfig& config,
                           const userver::components::ComponentContext& context)
    : ComponentBase(config, context),
      pool_(context.FindComponent<userver::components::Mongo>("mongo-items").GetPool()) {}

std::string ItemsStorage::AddItem(const std::string& name,
                                   const std::string& description,
                                   double price, int quantity) {
    auto col = pool_->GetCollection("items");
    auto oid = userver::formats::bson::Oid{};  // генерируется автоматически
    col.InsertOne(MakeDoc(
        "_id", oid,
        "name", name,
        "description", description,
        "price", price,
        "quantity", quantity
    ));
    return oid.ToString();
}

std::optional<Item> ItemsStorage::GetItem(const std::string& id) const {
    auto col = pool_->GetCollection("items");
    auto oid = userver::formats::bson::Oid{id};
    auto result = col.FindOne(MakeDoc("_id", oid));
    if (!result) return std::nullopt;

    return Item{
        (*result)["_id"].As<userver::formats::bson::Oid>().ToString(),
        (*result)["name"].As<std::string>(),
        (*result)["description"].As<std::string>(),
        (*result)["price"].As<double>(),
        (*result)["quantity"].As<int>()
    };
}

std::vector<Item> ItemsStorage::GetAllItems() const {
    auto col = pool_->GetCollection("items");
    auto cursor = col.Find({});
    std::vector<Item> items;
    for (const auto& doc : cursor) {
        items.push_back(Item{
            doc["_id"].As<userver::formats::bson::Oid>().ToString(),
            doc["name"].As<std::string>(),
            doc["description"].As<std::string>(),
            doc["price"].As<double>(),
            doc["quantity"].As<int>()
        });
    }
    return items;
}

bool ItemsStorage::UpdateItem(const std::string& id,
                               const std::optional<std::string>& name,
                               const std::optional<std::string>& description,
                               const std::optional<double>& price,
                               const std::optional<int>& quantity) {
    auto col = pool_->GetCollection("items");
    auto oid = userver::formats::bson::Oid{id};

    userver::formats::bson::ValueBuilder set_doc;
    if (name)        set_doc["name"] = *name;
    if (description) set_doc["description"] = *description;
    if (price)       set_doc["price"] = *price;
    if (quantity)    set_doc["quantity"] = *quantity;

    auto result = col.UpdateOne(
        MakeDoc("_id", oid),
        MakeDoc("$set", set_doc.ExtractValue())
    );
    return result.MatchedCount() > 0;
}

bool ItemsStorage::DeleteItem(const std::string& id) {
    auto col = pool_->GetCollection("items");
    auto oid = userver::formats::bson::Oid{id};
    auto result = col.DeleteOne(MakeDoc("_id", oid));
    return result.DeletedCount() > 0;
}

} // namespace item_storage
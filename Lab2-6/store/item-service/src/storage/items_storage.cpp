#include "items_storage.hpp"
#include <userver/components/component_context.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/formats/bson/inline.hpp>
#include <userver/formats/bson/value_builder.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/logging/log.hpp>
#include <userver/storages/secdist/component.hpp>
#include <userver/storages/secdist/provider_component.hpp>

namespace item_service {

using userver::formats::bson::MakeDoc;

namespace {

std::string ItemsToJson(const std::vector<Item>& items) {
    userver::formats::json::ValueBuilder arr(userver::formats::json::Type::kArray);
    for (const auto& item : items) {
        userver::formats::json::ValueBuilder obj;
        obj["id"]          = item.id;
        obj["name"]        = item.name;
        obj["description"] = item.description;
        obj["price"]       = item.price;
        obj["quantity"]    = item.quantity;
        arr.PushBack(obj.ExtractValue());
    }
    return userver::formats::json::ToString(arr.ExtractValue());
}

std::vector<Item> ItemsFromJson(const std::string& json_str) {
    auto root = userver::formats::json::FromString(json_str);
    std::vector<Item> items;
    for (const auto& obj : root) {
        items.push_back(Item{
            obj["id"].As<std::string>(),
            obj["name"].As<std::string>(),
            obj["description"].As<std::string>(),
            obj["price"].As<double>(),
            obj["quantity"].As<int>()
        });
    }
    return items;
}

}

ItemsStorage::ItemsStorage(const userver::components::ComponentConfig& config,
                           const userver::components::ComponentContext& context)
    : ComponentBase(config, context),
      pool_(context.FindComponent<userver::components::Mongo>("mongo-items").GetPool()),
      redis_client_(
          context.FindComponent<userver::components::Redis>("items-cache")
              .GetClient("items-cache")
      ),
      redis_cc_{std::chrono::seconds{5}, std::chrono::seconds{15}, 3} {}


void ItemsStorage::InvalidateItemListCache() const {
    try {
        redis_client_->Del(std::string{kItemListCacheKey}, redis_cc_).Get();
        LOG_DEBUG() << "Item list cache invalidated";
    } catch (const std::exception& e) {
        LOG_WARNING() << "Failed to invalidate item list cache: " << e.what();
    }
}

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
    InvalidateItemListCache();
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
    try {
        const auto cached = redis_client_->Get(
            std::string{kItemListCacheKey}, redis_cc_).Get();
        if (cached) {
            LOG_DEBUG() << "Item list cache HIT";
            return ItemsFromJson(*cached);
        }
        LOG_DEBUG() << "Item list cache MISS";
    } catch (const std::exception& e) {
        LOG_WARNING() << "Redis GET failed, falling back to MongoDB: " << e.what();
    }

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

    try {
        const auto json_str = ItemsToJson(items);
        redis_client_->Set(std::string{kItemListCacheKey}, json_str, redis_cc_).Get();
        redis_client_->Expire(std::string{kItemListCacheKey},std::chrono::seconds{kItemListTtlSeconds},redis_cc_).Get();
        LOG_DEBUG() << "Item list cached (" << items.size() << " items)";
    } catch (const std::exception& e) {
        LOG_WARNING() << "Failed to cache item list: " << e.what();
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

    if (result.MatchedCount() > 0) InvalidateItemListCache();
    return result.MatchedCount() > 0;
}

bool ItemsStorage::DeleteItem(const std::string& id) {
    auto col = pool_->GetCollection("items");
    auto oid = userver::formats::bson::Oid{id};
    auto result = col.DeleteOne(MakeDoc("_id", oid));
    if (result.DeletedCount() > 0) InvalidateItemListCache();
    return result.DeletedCount() > 0;
}

} // namespace item_storage
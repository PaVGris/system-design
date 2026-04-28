#include <handlers/items.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/components/component_context.hpp>
#include <schemas/item.hpp>

namespace item_service {

namespace {

userver::formats::json::Value ItemToJson(const Item& item) {
    userver::formats::json::ValueBuilder builder;
    builder["id"] = item.id;
    builder["name"] = item.name;
    builder["description"] = item.description;
    builder["price"] = item.price;
    builder["quantity"] = item.quantity;
    return builder.ExtractValue();
}

}  // namespace

GetItemsHandler::GetItemsHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<ItemsStorage>()) {}

std::string GetItemsHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    const auto items = storage_.GetAllItems();

    userver::formats::json::ValueBuilder result;
    result = userver::formats::json::ValueBuilder(
        userver::formats::json::Type::kArray);

    for (const auto& item : items) {
        result.PushBack(ItemToJson(item));
    }

    request.GetHttpResponse().SetContentType("application/json");
    return userver::formats::json::ToString(result.ExtractValue());
}

GetItemHandler::GetItemHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<ItemsStorage>()) {}

std::string GetItemHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    const std::string id = request.GetPathArg("id");

    std::optional<Item> item;
    try {
        item = storage_.GetItem(id);
    } catch (const std::exception&) {
        throw userver::server::handlers::ResourceNotFound();
    }

    if (!item) {
        throw userver::server::handlers::ResourceNotFound();
    }

    request.GetHttpResponse().SetContentType("application/json");
    return userver::formats::json::ToString(ItemToJson(*item));
}

CreateItemHandler::CreateItemHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<ItemsStorage>()) {}

std::string CreateItemHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    auto body = userver::formats::json::FromString(request.RequestBody());
    auto req = body.As<CreateItemRequest>();

    const std::string description = req.description.value_or("");
    const std::string id = storage_.AddItem(req.name, description, req.price, req.quantity);

    userver::formats::json::ValueBuilder result;
    result["id"] = id;

    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kCreated);
    request.GetHttpResponse().SetContentType("application/json");
    return userver::formats::json::ToString(result.ExtractValue());
}

UpdateItemHandler::UpdateItemHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<ItemsStorage>()) {}

std::string UpdateItemHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    const std::string id =  request.GetPathArg("id");
    auto body = userver::formats::json::FromString(request.RequestBody());
    auto req = body.As<UpdateItemRequest>();

    const bool updated = storage_.UpdateItem(
        id, req.name, req.description, req.price, req.quantity);

    if (!updated) {
        throw userver::server::handlers::ResourceNotFound();
    }

    request.GetHttpResponse().SetContentType("application/json");
    return R"({"status": "ok"})";
}

DeleteItemHandler::DeleteItemHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<ItemsStorage>()) {}

std::string DeleteItemHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    const std::string id =  request.GetPathArg("id");
    const bool deleted = storage_.DeleteItem(id);

    if (!deleted) {
        throw userver::server::handlers::ResourceNotFound();
    }

    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kNoContent);
    return "";
}

}  // namespace item_service
#include <handlers/cart.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/components/component_context.hpp>
#include <userver/components/component_config.hpp>
#include <userver/clients/http/component.hpp>
#include <schemas/cart.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

namespace cart_service {

GetCartHandler::GetCartHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<CartStorage>()) {}

std::string GetCartHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    const auto& user_id_str = request.GetHeader("X-User-Id");
    if (user_id_str.empty()) {
        throw userver::server::handlers::Unauthorized(
            userver::server::handlers::ExternalBody{"Missing user id"});
    }
    const int user_id = std::stoi(std::string(user_id_str));

    const auto items = storage_.GetCart(user_id);

    userver::formats::json::ValueBuilder result;
    result["user_id"] = user_id;
    result["items"] = userver::formats::json::ValueBuilder(
        userver::formats::json::Type::kArray);

    for (const auto& item : items) {
        userver::formats::json::ValueBuilder item_json;
        item_json["item_id"] = item.item_id.value_or(0);
        item_json["quantity"] = item.quantity.value_or(0);
        result["items"].PushBack(item_json.ExtractValue());
    }

    request.GetHttpResponse().SetContentType("application/json");
    return userver::formats::json::ToString(result.ExtractValue());
}

AddToCartHandler::AddToCartHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<CartStorage>()),
      http_client_(context.FindComponent<userver::components::HttpClient>().GetHttpClient()),
      item_service_url_(config["item-service-url"].As<std::string>()) {}

std::string AddToCartHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    const auto& user_id_str = request.GetHeader("X-User-Id");
    if (user_id_str.empty()) {
        throw userver::server::handlers::Unauthorized(
            userver::server::handlers::ExternalBody{"Missing user id"});
    }
    const int user_id = std::stoi(std::string(user_id_str));

    auto body = userver::formats::json::FromString(request.RequestBody());
    auto req = body.As<AddToCartRequest>();

    auto item_response = http_client_.CreateRequest()
        .get(item_service_url_ + "/items/" + std::to_string(req.item_id))
        .timeout(std::chrono::seconds{5})
        .perform();

    if (item_response->status_code() == 404) {
        throw userver::server::handlers::ResourceNotFound();
    }

    auto item_json = userver::formats::json::FromString(item_response->body());
    const int available = item_json["quantity"].As<int>();

    if (req.quantity > available) {
        throw userver::server::handlers::ClientError(
            userver::server::handlers::ExternalBody{
                "Not enough items in stock"});
    }
        storage_.AddItem(user_id, req.item_id, req.quantity);

    request.GetHttpResponse().SetContentType("application/json");
    return R"({"status": "ok"})";
}

userver::yaml_config::Schema AddToCartHandler::GetStaticConfigSchema() {
    return userver::yaml_config::MergeSchemas<userver::server::handlers::HttpHandlerBase>(R"(
type: object
description: Add to cart handler
additionalProperties: false
properties:
    item-service-url:
        type: string
        description: URL of item service
)");
}

RemoveFromCartHandler::RemoveFromCartHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<CartStorage>()) {}

std::string RemoveFromCartHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    const auto& user_id_str = request.GetHeader("X-User-Id");
    if (user_id_str.empty()) {
        throw userver::server::handlers::Unauthorized(
            userver::server::handlers::ExternalBody{"Missing user id"});
    }
    const int user_id = std::stoi(std::string(user_id_str));
    const int item_id = std::stoi(request.GetPathArg("item_id"));

    const bool removed = storage_.RemoveItem(user_id, item_id);
    if (!removed) {
        throw userver::server::handlers::ResourceNotFound();
    }

    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kNoContent);
    return "";
}

}  // namespace cart_service
#include <handlers/cart.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/components/component_context.hpp>
#include <userver/components/component_config.hpp>
#include <userver/clients/http/component.hpp>
#include <schemas/cart.hpp>
#include <userver/yaml_config/merge_schemas.hpp>
#include <userver/logging/log.hpp>  
#include <userver/kafka/producer_component.hpp> 

namespace cart_service {

GetCartHandler::GetCartHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<cart_service::CartStorage>()) {}

std::string GetCartHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    const auto& user_id_str = request.GetHeader("X-User-Id");
    if (user_id_str.empty()) {
        throw userver::server::handlers::Unauthorized(
            userver::server::handlers::ExternalBody{"Missing user id"});
    }
    const int user_id = std::stoi(user_id_str);

    const auto db_items = storage_.GetItems(user_id);

    cart_service::Cart response;
    response.user_id = user_id;
    response.items.emplace(); 

    for (const auto& db_item : db_items) {
        cart_service::CartItem api_item;
        api_item.item_id = db_item.item_id;
        api_item.quantity = db_item.quantity;

        response.items->push_back(std::move(api_item));
    }

    request.GetHttpResponse().SetContentType("application/json");
    return userver::formats::json::ToString(
        userver::formats::json::ValueBuilder{response}.ExtractValue()
    );
}
AddToCartHandler::AddToCartHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<cart_service::CartStorage>()),
      http_client_(context.FindComponent<userver::components::HttpClient>().GetHttpClient()),
      item_service_url_(config["item-service-url"].As<std::string>()),
      producer_(context.FindComponent<userver::kafka::ProducerComponent>(
                    "kafka-producer-cart")
                    .GetProducer()) {}

std::string AddToCartHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    const auto& user_id_str = request.GetHeader("X-User-Id");
    if (user_id_str.empty()) {
        throw userver::server::handlers::Unauthorized(
            userver::server::handlers::ExternalBody{"Missing user id"});
    }
    const int user_id = std::stoi(user_id_str);

    auto req = userver::formats::json::FromString(request.RequestBody()).As<AddToCartRequest>();

    auto item_response = http_client_.CreateRequest()
        .get(item_service_url_ + "/items/" + req.item_id)
        .timeout(std::chrono::seconds{5})
        .perform();

    if (item_response->status_code() == 404) {
        throw userver::server::handlers::ResourceNotFound();
    }

    auto item_json = userver::formats::json::FromString(item_response->body());
    if (req.quantity > item_json["quantity"].As<int>()) {
        throw userver::server::handlers::ClientError(
            userver::server::handlers::ExternalBody{"Not enough items in stock"});
    }

    storage_.AddItem(user_id, req.item_id, req.quantity);

    try {
        userver::formats::json::ValueBuilder event;
        event["event_type"] = "ItemAddedToCart";
        event["user_id"]    = user_id;
        event["item_id"]    = req.item_id;
        event["quantity"]   = req.quantity;

        producer_.Send(
            "cart-events",
            std::to_string(user_id),
            userver::formats::json::ToString(event.ExtractValue())
        );
        LOG_INFO() << "Published ItemAddedToCart event for user_id=" << user_id
                   << " item_id=" << req.item_id;
    } catch (const userver::kafka::SendException& ex) {
        LOG_ERROR() << "Failed to publish ItemAddedToCart event: " << ex.what();
    }

    request.GetHttpResponse().SetContentType("application/json");
    return R"({"status": "ok"})";
}

RemoveFromCartHandler::RemoveFromCartHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<cart_service::CartStorage>()) {}

std::string RemoveFromCartHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    const auto& user_id_str = request.GetHeader("X-User-Id");
    if (user_id_str.empty()) {
        throw userver::server::handlers::Unauthorized(
            userver::server::handlers::ExternalBody{"Missing user id"});
    }
    const int user_id = std::stoi(user_id_str);
    const std::string item_id =  request.GetPathArg("item_id");
    storage_.RemoveItem(user_id, item_id);

    request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kNoContent);
    return "";
}

userver::yaml_config::Schema AddToCartHandler::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<userver::server::handlers::HttpHandlerBase>(R"(
type: object
description: AddToCartHandler handler schema
additionalProperties: false
properties:
    item-service-url:
        type: string
        description: url of item-service
)");
}

} // namespace cart_service
#include <handlers/gateway.hpp>
#include <userver/components/component_context.hpp>
#include <userver/components/component_config.hpp>
#include <userver/clients/http/component.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/yaml_config/merge_schemas.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/formats/json/value_builder.hpp>

namespace gateway {

GatewayHandler::GatewayHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      http_client_(context.FindComponent<userver::components::HttpClient>()
                       .GetHttpClient()),
      user_service_url_(config["user-service-url"].As<std::string>()),
      item_service_url_(config["item-service-url"].As<std::string>()),
      cart_service_url_(config["cart-service-url"].As<std::string>()) {}

std::string GatewayHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    const auto& path = request.GetUrl();
    const auto& method = request.GetMethod();

    const bool is_public =
        (path == "/users/register" || path == "/users/login");

    int user_id = 0;
    std::string role;

    if (!is_public) {
        const auto& auth = request.GetHeader("Authorization");
        if (auth.empty()) {
            request.GetHttpResponse().SetStatus(
                userver::server::http::HttpStatus::kUnauthorized);
            request.GetHttpResponse().SetContentType("application/json");
            return R"({"message": "Missing token"})";
        }

        auto validate_response = http_client_.CreateRequest()
            .get(user_service_url_ + "/users/validate")
            .headers({{"Authorization", std::string(auth)}})
            .timeout(std::chrono::seconds{5})
            .perform();

        if (validate_response->status_code() == 401) {
            request.GetHttpResponse().SetStatus(
                userver::server::http::HttpStatus::kUnauthorized);
            request.GetHttpResponse().SetContentType("application/json");
            return R"({"message": "Invalid token"})";
        }

        auto json = userver::formats::json::FromString(
            validate_response->body());
        user_id = json["user_id"].As<int>();
        role = json["role"].As<std::string>();

        if (path.find("/items") == 0 &&
            (method == userver::server::http::HttpMethod::kPost ||
             method == userver::server::http::HttpMethod::kPut ||
             method == userver::server::http::HttpMethod::kDelete)) {
            if (role != "seller") {
                request.GetHttpResponse().SetStatus(
                    userver::server::http::HttpStatus::kForbidden);
                request.GetHttpResponse().SetContentType("application/json");
                return R"({"message": "Forbidden: seller role required"})";
            }
        }
    }

    std::string target_url;
    if (path.find("/users") == 0) {
        target_url = user_service_url_ + path;
    } else if (path.find("/items") == 0) {
        target_url = item_service_url_ + path;
    } else if (path.find("/cart") == 0) {
        target_url = cart_service_url_ + path;
    } else {
        throw userver::server::handlers::ResourceNotFound();
    }
    
    auto req = http_client_.CreateRequest()
        .url(target_url)
        .timeout(std::chrono::seconds{10});

    const auto& auth = request.GetHeader("Authorization");
    if (!auth.empty()) {
        req.headers({
            {"Authorization", std::string_view(auth)},
            {"X-User-Id", std::string_view(std::to_string(user_id))},
            {"X-User-Role", std::string_view(role)}
        });
    }

    if (method == userver::server::http::HttpMethod::kPost) {
        req.post(target_url, request.RequestBody());
    } else if (method == userver::server::http::HttpMethod::kPut) {
        req.put(target_url, request.RequestBody());
    } else if (method == userver::server::http::HttpMethod::kDelete) {
        req.delete_method();
    } else {
        req.get(target_url);
    }

    auto response = req.perform();

    request.GetHttpResponse().SetStatus(
        static_cast<userver::server::http::HttpStatus>(
            response->status_code()));
    request.GetHttpResponse().SetContentType("application/json");
    return response->body();
}

userver::yaml_config::Schema GatewayHandler::GetStaticConfigSchema() {
    return userver::yaml_config::MergeSchemas<userver::server::handlers::HttpHandlerBase>(R"(
        type: object
        description: Gateway handler
        additionalProperties: false
        properties:
            user-service-url:
                type: string
                description: URL of user service
            item-service-url:
                type: string
                description: URL of item service
            cart-service-url:
                type: string
                description: URL of cart service
        )");
}

}  // namespace gateway
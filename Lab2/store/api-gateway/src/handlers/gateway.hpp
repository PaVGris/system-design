#pragma once
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/clients/http/client.hpp>
#include <userver/yaml_config/schema.hpp>

namespace gateway {

class GatewayHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    // static constexpr std::string_view kName = "handler-gateway";

    GatewayHandler(const userver::components::ComponentConfig& config,
                   const userver::components::ComponentContext& context);

    static userver::yaml_config::Schema GetStaticConfigSchema();

    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest& request,
        userver::server::request::RequestContext&
    ) const override;

private:
    userver::clients::http::Client& http_client_;
    std::string user_service_url_;
    std::string item_service_url_;
    std::string cart_service_url_;
};

}  // namespace gateway
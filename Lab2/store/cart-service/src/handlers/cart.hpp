#pragma once
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/clients/http/client.hpp>
#include <userver/yaml_config/schema.hpp>
#include <userver/components/component_base.hpp>
#include <storage/cart_storage.hpp>

namespace cart_service {

class GetCartHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-get-cart";

    GetCartHandler(const userver::components::ComponentConfig& config,
                   const userver::components::ComponentContext& context);

    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest& request,
        userver::server::request::RequestContext&
    ) const override;

private:
    CartStorage& storage_;
};

class AddToCartHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-add-to-cart";

    AddToCartHandler(const userver::components::ComponentConfig& config,
                     const userver::components::ComponentContext& context);

    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest& request,
        userver::server::request::RequestContext&
    ) const override;

    static userver::yaml_config::Schema GetStaticConfigSchema();

private:
    CartStorage& storage_;
    userver::clients::http::Client& http_client_;
    std::string item_service_url_;
};

class RemoveFromCartHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-remove-from-cart";

    RemoveFromCartHandler(const userver::components::ComponentConfig& config,
                          const userver::components::ComponentContext& context);

    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest& request,
        userver::server::request::RequestContext&
    ) const override;

private:
    CartStorage& storage_;
};

}  // namespace cart_service
#pragma once
#include <userver/server/handlers/http_handler_base.hpp>
#include <storage/items_storage.hpp>

namespace item_service {

class GetItemsHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-get-items";

    GetItemsHandler(const userver::components::ComponentConfig& config,
                    const userver::components::ComponentContext& context);

    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest& request,
        userver::server::request::RequestContext&
    ) const override;

private:
    ItemsStorage& storage_;
};

class GetItemHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-get-item";

    GetItemHandler(const userver::components::ComponentConfig& config,
                   const userver::components::ComponentContext& context);

    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest& request,
        userver::server::request::RequestContext&
    ) const override;

private:
    ItemsStorage& storage_;
};

class CreateItemHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-create-item";

    CreateItemHandler(const userver::components::ComponentConfig& config,
                      const userver::components::ComponentContext& context);

    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest& request,
        userver::server::request::RequestContext&
    ) const override;

private:
    ItemsStorage& storage_;
};

class UpdateItemHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-update-item";

    UpdateItemHandler(const userver::components::ComponentConfig& config,
                      const userver::components::ComponentContext& context);

    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest& request,
        userver::server::request::RequestContext&
    ) const override;

private:
    ItemsStorage& storage_;
};

class DeleteItemHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-delete-item";

    DeleteItemHandler(const userver::components::ComponentConfig& config,
                      const userver::components::ComponentContext& context);

    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest& request,
        userver::server::request::RequestContext&
    ) const override;

private:
    ItemsStorage& storage_;
};

}  // namespace item_service
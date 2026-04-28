#include <userver/components/minimal_server_component_list.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component_list.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <handlers/items.hpp>
#include <storage/items_storage.hpp>
#include <userver/storages/mongo/component.hpp>

int main(int argc, char* argv[]) {
    auto component_list =
        userver::components::MinimalServerComponentList()
            .Append<userver::components::Mongo>("mongo-items")
            .Append<userver::components::TestsuiteSupport>()
            .Append<item_service::ItemsStorage>()
            .Append<item_service::GetItemsHandler>()
            .Append<item_service::GetItemHandler>()
            .Append<item_service::CreateItemHandler>()
            .Append<item_service::UpdateItemHandler>()
            .Append<item_service::DeleteItemHandler>()
            .Append<userver::server::handlers::Ping>()
            .AppendComponentList(userver::clients::http::ComponentList())
            .Append<userver::clients::dns::Component>()
            .Append<userver::server::handlers::TestsControl>();

    return userver::utils::DaemonMain(argc, argv, component_list);
}
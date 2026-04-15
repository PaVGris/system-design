#include <userver/components/minimal_server_component_list.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component_list.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <handlers/auth.hpp>
#include <storage/users_storage.hpp>
#include <userver/storages/postgres/component.hpp>

int main(int argc, char* argv[]) {
    auto component_list =
        userver::components::MinimalServerComponentList()
            .Append<userver::components::Postgres>("postgres-db")
            .Append<user_service::UsersStorage>()
            .Append<userver::components::TestsuiteSupport>()
            .Append<user_service::RegisterHandler>()
            .Append<user_service::LoginHandler>()
            .Append<user_service::GetUserHandler>()
            .Append<user_service::ValidateTokenHandler>()
            .Append<userver::server::handlers::Ping>()
            .AppendComponentList(userver::clients::http::ComponentList())
            .Append<userver::clients::dns::Component>()
            .Append<userver::server::handlers::TestsControl>();

    return userver::utils::DaemonMain(argc, argv, component_list);
}
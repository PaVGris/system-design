#include <userver/components/minimal_server_component_list.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component_list.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <handlers/gateway.hpp>

int main(int argc, char* argv[]) {
    auto component_list =
        userver::components::MinimalServerComponentList()
            .Append<userver::components::TestsuiteSupport>()
            .Append<gateway::GatewayHandler>("handler-gateway-users")
            .Append<gateway::GatewayHandler>("handler-gateway-items")
            .Append<gateway::GatewayHandler>("handler-gateway-items-root")
            .Append<gateway::GatewayHandler>("handler-gateway-cart")
            .Append<gateway::GatewayHandler>("handler-gateway-cart-root")
            .Append<userver::server::handlers::Ping>()
            .AppendComponentList(userver::clients::http::ComponentList())
            .Append<userver::clients::dns::Component>()
            .Append<userver::server::handlers::TestsControl>();

    return userver::utils::DaemonMain(argc, argv, component_list);
}
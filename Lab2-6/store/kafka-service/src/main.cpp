#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component_list.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/kafka/consumer_component.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <userver/storages/secdist/component.hpp>
#include <userver/storages/secdist/provider_component.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>

#include "handlers/consumer_handler.hpp"

int main(int argc, char* argv[]) {
    auto component_list =
        userver::components::MinimalServerComponentList()
            .Append<userver::kafka::ConsumerComponent>("kafka-consumer")
            .Append<userver::components::Secdist>()
            .Append<userver::components::DefaultSecdistProvider>()
            .Append<userver::components::TestsuiteSupport>()
            .Append<kafka_service::ConsumerHandler>()
            .Append<userver::server::handlers::Ping>()
            .AppendComponentList(userver::clients::http::ComponentList())
            .Append<userver::clients::dns::Component>()
            .Append<userver::server::handlers::TestsControl>();

    return userver::utils::DaemonMain(argc, argv, component_list);
}

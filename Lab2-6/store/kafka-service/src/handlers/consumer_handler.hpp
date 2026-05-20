#pragma once

#include <userver/components/component_base.hpp>
#include <userver/kafka/consumer_scope.hpp>

namespace kafka_service {

class ConsumerHandler final : public userver::components::ComponentBase {
public:
    static constexpr std::string_view kName{"consumer-handler"};

    ConsumerHandler(const userver::components::ComponentConfig& config,
                    const userver::components::ComponentContext& context);

    ~ConsumerHandler() override;

private:
    void Consume(userver::kafka::MessageBatchView messages);
    userver::kafka::ConsumerScope consumer_;
};

}  // namespace kafka_service

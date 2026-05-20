#pragma once

#include <userver/components/component_base.hpp>
#include <userver/kafka/consumer_scope.hpp>

namespace kafka_service {

// Компонент-consumer: запускается при старте и читает события из топиков.
// Подписан на: user-events, item-events, cart-events
class ConsumerHandler final : public userver::components::ComponentBase {
public:
    static constexpr std::string_view kName{"consumer-handler"};

    ConsumerHandler(const userver::components::ComponentConfig& config,
                    const userver::components::ComponentContext& context);

    ~ConsumerHandler() override;

private:
    void Consume(userver::kafka::MessageBatchView messages);

    // Subscriptions must be the last field!
    userver::kafka::ConsumerScope consumer_;
};

}  // namespace kafka_service

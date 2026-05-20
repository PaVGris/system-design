#include "consumer_handler.hpp"

#include <userver/components/component_context.hpp>
#include <userver/kafka/consumer_component.hpp>      
#include <userver/logging/log.hpp>

namespace kafka_service {

ConsumerHandler::ConsumerHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : userver::components::ComponentBase{config, context},
      consumer_{context.FindComponent<userver::kafka::ConsumerComponent>(
                    "kafka-consumer")
                    .GetConsumer()} {

    consumer_.Start([this](userver::kafka::MessageBatchView messages) {
        Consume(messages);
        consumer_.AsyncCommit();
    });
}

ConsumerHandler::~ConsumerHandler() {
    consumer_.Stop();
}

void ConsumerHandler::Consume(userver::kafka::MessageBatchView messages) {
    for (const auto& message : messages) {
        const auto topic   = message.GetTopic();
        const auto key     = message.GetKey();
        const auto payload = message.GetPayload();

        LOG_INFO() << "[consumer] topic=" << topic
                   << " key=" << key
                   << " payload=" << payload;

        // Маршрутизация по топику
        if (topic == "user-events") {
            LOG_INFO() << "[consumer] Обработка UserEvent: " << payload;
            // Например: обновление read-модели пользователей
        } else if (topic == "item-events") {
            LOG_INFO() << "[consumer] Обработка ItemEvent: " << payload;
            // Например: инвалидация кеша каталога
        } else if (topic == "cart-events") {
            LOG_INFO() << "[consumer] Обработка CartEvent: " << payload;
            // Например: обновление счётчика резервирования товаров
        }
    }
}

}  // namespace kafka_service

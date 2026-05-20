# Event-Driven архитектура

## 1. События и команды

| Команда (HTTP) | Событие | Топик |
|---|---|---|
| `POST /users/register` | `UserRegistered` | `user-events` |
| `POST /items` | `ItemCreated` | `item-events` |
| `POST /cart` | `ItemAddedToCart` | `cart-events` |

## 2. Компоненты системы

```
┌─────────────────┐        ┌─────────────────┐        ┌─────────────────┐
│  user-service   │        │  item-service   │        │  cart-service   │
│                 │        │                 │        │                 │
│  [Producer]     │        │  [Producer]     │        │  [Producer]     │
│  user-events    │        │  item-events    │        │  cart-events    │
└────────┬────────┘        └────────┬────────┘        └────────┬────────┘
         │                          │                           │
         └──────────────────────────┼───────────────────────────┘
                                    │
                          ┌─────────▼─────────┐
                          │   Kafka Cluster    │
                          │  (kafka1, kafka2)  │
                          │                    │
                          │  • user-events     │
                          │  • item-events     │
                          │  • cart-events     │
                          └─────────┬──────────┘
                                    │
                          ┌─────────▼──────────┐
                          │   kafka-service    │
                          │   [Consumer]       │
                          │  group: store-     │
                          │  consumer-group    │
                          └────────────────────┘
```

**Producers** — каждый сервис публикует событие непосредственно после успешной записи в свою БД:
- `user-service` → `user-events`
- `item-service` → `item-events`
- `cart-service` → `cart-events`

**Consumer** — `kafka-service` подписан на все три топика и обрабатывает события централизованно.

## 3. Брокер сообщений

Выбран **Apache Kafka** как более подходящий для event-driven архитектуры: сохраняет лог событий, поддерживает replay, горизонтально масштабируется.

Кластер запускается в режиме **KRaft** (без Zookeeper) из двух брокеров:
- `kafka1` — broker id 1
- `kafka2` — broker id 2

Оба брокера одновременно являются контроллерами (`KAFKA_CFG_PROCESS_ROLES: broker,controller`), кворум: `1@kafka1:9093,2@kafka2:9093`.

**Интерфейсы подключения:**
- **INTERNAL:** порт `19092`. Используется для взаимодействия микросервисов и `kafka-service` внутри изолированной докер-сети.
- **EXTERNAL:** порты `9092` (для `kafka1`) и `9094` (для `kafka2`). Проброшены на хост-машину (`localhost`) для локальной отладки и подключения `kafka-ui`.

**Гарантии доставки:** `at-least-once` — producer настроен с `acks=all` и `enable_idempotence=true`, что устраняет дубликаты при повторных отправках от одного producer. Consumer делает `AsyncCommit` только после успешной обработки батча.

## 4. CQRS

CQRS применим в данной системе частично.

**Write-модель** — каждый сервис пишет в свою БД синхронно (PostgreSQL для пользователей, MongoDB для товаров и корзины).

**Read-модель** — `kafka-service` потребляет события и может строить агрегированные представления: например, счётчик товаров в корзинах, список зарегистрированных пользователей за период, статистика по товарам. В текущей реализации read-модель не материализована (только логирование), но архитектура позволяет добавить отдельное хранилище без изменения producers.

**Синхронизация моделей:** событие публикуется сразу после успешной записи в write-БД. Consumer обрабатывает события асинхронно — возможна eventual consistency с небольшой задержкой.

## 5. Поток событий

```
Client
  │
  ▼
POST /users/register
  │
  ├─► UsersStorage.AddUser() ──► PostgreSQL
  │
  └─► kafka::Producer.Send("user-events", user_id, payload)
            │
            ▼
        Kafka Cluster
            │
            ▼
        kafka-service ConsumerHandler.Consume()
            └─► LOG_INFO (обработка UserRegistered)
```

Аналогично для `item-events` (триггер: `POST /items`) и `cart-events` (триггер: `POST /cart`).

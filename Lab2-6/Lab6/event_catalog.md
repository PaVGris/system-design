# Каталог событий

## UserRegistered

| Поле | Значение |
|---|---|
| **Топик** | `user-events` |
| **Производитель** | `user-service` |
| **Потребители** | `kafka-service` |
| **Ключ партиции** | `user_id` |
| **Гарантия доставки** | at-least-once |
| **Триггер** | `POST /users/register` |

**Payload:**
```json
{
  "event_type": "UserRegistered",
  "user_id": 42,
  "username": "pavel",
  "name": "Pavel Ivanov",
  "role": "client"
}
```

---

## ItemCreated

| Поле | Значение |
|---|---|
| **Топик** | `item-events` |
| **Производитель** | `item-service` |
| **Потребители** | `kafka-service` |
| **Ключ партиции** | `item_id` |
| **Гарантия доставки** | at-least-once |
| **Триггер** | `POST /items` |

**Payload:**
```json
{
  "event_type": "ItemCreated",
  "item_id": "64f1a2b3c4d5e6f7a8b9c0d1",
  "name": "Laptop",
  "price": 999.99,
  "quantity": 10
}
```

---

## ItemAddedToCart

| Поле | Значение |
|---|---|
| **Топик** | `cart-events` |
| **Производитель** | `cart-service` |
| **Потребители** | `kafka-service` |
| **Ключ партиции** | `user_id` (все события одного пользователя — в одну партицию) |
| **Гарантия доставки** | at-least-once |
| **Триггер** | `POST /cart` |

**Payload:**
```json
{
  "event_type": "ItemAddedToCart",
  "user_id": 42,
  "item_id": "64f1a2b3c4d5e6f7a8b9c0d1",
  "quantity": 2
}
```

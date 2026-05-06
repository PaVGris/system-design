# Performance Design

## 1. Анализ производительности

### Hot paths (наиболее частые операции)

| Endpoint | Сервис | Почему горячий |
|---|---|---|
| `GET /users/validate` | user-service | Вызывается при каждом запросе через api-gateway |
| `GET /items` | item-service | Просмотр каталога ( частая операция каждого клиента) |
| `POST /users/login` | user-service | Потенциально высокий трафик (публичный endpoint) |

### Медленные операции (обращения к БД)

| Операция | Хранилище | Ожидаемая латентность |
|---|---|---|
| `SELECT user_id FROM tokens WHERE token = $1` | PostgreSQL | 1–5 мс (индекс есть, но сеть + disk I/O) |
| `db.items.find({})` | MongoDB | 2–10 мс (зависит от размера коллекции) |
| `db.items.findOne({"_id": oid})` | MongoDB | 1–3 мс |

### Требования к производительности

- `GET /users/validate` — < 2 мс с кешем (было 1–5 мс без него)
- `GET /items` — < 5 мс с кешем (было 2–10 мс без него)
- `POST /users/login` — не более 100 запросов в минуту с одного IP

---

## 2. Стратегия кеширования

### 2.1 Кеш токенов (`user-service`)

**Что кешируем:** маппинг `token → user_id`

**Почему именно это:** `/users/validate` вызывается при каждом запросе от клиента (api-gateway проверяет каждый входящий запрос). При 1000 RPS это 1000 лишних SQL-запросов в секунду.

**Стратегия:** Cache-Aside (Lazy Loading)

```
FindUserByToken(token):
  1. GET "token:{token}" из Redis
     HIT  → вернуть user_id                       
     MISS → SELECT ... FROM tokens WHERE token=$1
         → SET "token:{token}" user_id EX 3600
         → вернуть user_id
```

**Ключ:** `token:{hex_value}`  
**Значение:** строка с `user_id`  
**TTL:** 3600 секунд (1 час) — совпадает с желаемым временем жизни сессии  
**Инвалидация:** явная — при вызове `InvalidateToken()` ключ удаляется командой `DEL`

**Write-path (SaveToken):**
```
SaveToken(token, user_id):
  1. INSERT INTO tokens ...    # всегда пишем в Postgres (durability)
  2. SET "token:{token}" ...   # прогрев кеша сразу после логина
```

### 2.2 Кеш каталога товаров (`item-service`)

**Что кешируем:** весь список товаров `GET /items`

**Почему именно это:** каталог магазина содержит десятки товаров, читается при каждом открытии магазина, но меняется редко (продавцы добавляют/удаляют товары нечасто).

**Стратегия:** Cache-Aside с явной инвалидацией

```
GetAllItems():
  1. GET "items:all" из Redis
     HIT  → десериализовать JSON → вернуть []Item   
     MISS → db.items.find({}) из MongoDB             
         → сериализовать в JSON
         → SET "items:all" <json> EX 300
         → вернуть []Item
```

**Ключ:** `items:all`  
**Значение:** JSON-массив всех товаров  
**TTL:** 300 секунд (5 минут) — страховка от рассинхронизации  
**Инвалидация:** явная — при `AddItem`, `UpdateItem`, `DeleteItem` выполняется `DEL "items:all"`

**Почему не кешируем `GET /items/{id}`:**  
Запрашивается только при добавлении товара в корзину, количество операций мало. При этом `quantity` важно быть актуальным (чтобы не добавить в корзину товар, которого нет в наличии).

### 2.3 Кеш не нужен

| Данные | Причина |
|---|---|
| Корзина пользователя | Меняется при каждой операции добавления/удаления; риск показать устаревшее |
| Профиль пользователя (`GET /users/{id}`) | Редко запрашивается, данные должны быть актуальны |
| Результаты поиска по имени | Динамические запросы с масками, ключ тяжело нормализовать |

---

## 3. Конфигурация Redis

**Один экземпляр Redis** — shared между `user-service` и `item-service`.  
Разделение по namespace через префикс ключей:
- `token:*` — user-service
- `items:*` — item-service

**Параметры запуска:**
```
redis-server --maxmemory 128mb --maxmemory-policy allkeys-lru
```

`allkeys-lru` — при нехватке памяти вытесняются наименее недавно используемые ключи. Это безопасно, так как Redis является **кешем**, а не основным хранилищем: все данные есть в Postgres/MongoDB.

---

## 4. Проектирование Rate Limiting

### 4.1 Выбор endpoints

| Endpoint | Причина ограничения |
|---|---|
| `POST /users/login` | Brute-force атаки на пароли |
| `POST /users/register` | Спам-регистрации, исчерпание ID |
| `GET /items` | Защита от DDoS-краулеров (хотя есть кеш) |

### 4.2 Алгоритм: Sliding Window Counter (через Redis)

**Выбор обоснован:**
- Нет всплесков в пределах окна (в отличие от Fixed Window)
- Меньше памяти, чем Sliding Window Log (без хранения timestamp каждого запроса)
- Подходит для распределённой среды через Redis `INCR` + `EXPIRE`

**Реализация через Redis:**

```
RateLimit(ip, endpoint, limit, window_seconds):
  key = "rl:{endpoint}:{ip}:{unix_ts / window_seconds}"
  count = INCR key
  if count == 1:
    EXPIRE key window_seconds
  if count > limit:
    return 429 Too Many Requests
  return 200
```

### 4.3 Лимиты

| Endpoint | Лимит | Окно |
|---|---|---|
| `POST /users/login` | 10 запросов | 60 секунд |
| `POST /users/register` | 5 запросов | 60 секунд |
| `GET /items` | 60 запросов | 60 секунд |


## 5. Реализация rate limiting

Rate limiting реализован в `user-service` для endpoint `POST /users/login`.

**Алгоритм:** Fixed Window Counter через Redis `INCR` + `EXPIRE`.

**Реализация:** при каждом запросе инкрементируется счётчик `rate_limit:login:{ip}`.
При первом инкременте устанавливается TTL 60 секунд. Если счётчик превышает
лимит (10), возвращается `HTTP 429 Too Many Requests`.

**Почему Fixed Window вместо Sliding Window:**
для защиты от брутфорса точность окна не критична, а Fixed Window
значительно прощe.
```
При превышении — `HTTP 429 Too Many Requests`:
```json
{"error": "Too many requests. Please try again later."}
```

---

## 6. Влияние на производительность

### Снижение нагрузки на БД

| Операция | Без кеша | С кешем |
|---|---|---|
| Валидация токена (1000 RPS) | 1000 SQL/сек | ~10 SQL/сек (cache miss) | 
| GET /items (100 RPS) | 100 MongoDB/сек | ~0.3 MongoDB/сек |

### Снижение latency

| Endpoint | Без кеша | С кешем (HIT) |
|---|---|---|
| `/users/validate` | 1–5 мс | 0.3–0.5 мс |
| `GET /items` | 5–10 мс | 0.5–1 мс |

### Метрики для мониторинга

| Метрика | Инструмент | Целевое значение |
|---|---|---|
| Cache hit rate (токены) | Redis `INFO stats` → `keyspace_hits / (hits + misses)` | > 90% |
| Cache hit rate (items:all) | Redis keyspace_hits | > 95% |
| p99 latency `/users/validate` | userver prometheus метрики | < 2 мс |
| Redis memory usage | `redis-cli INFO memory` | < 100 MB |
| Rate limit hits / мин | Счётчик в gateway логах | Мониторинг аномалий |

### Измерение эффективности кеша (Hit Rate)

```bash
# Redis CLI: смотрим статистику hits/misses
redis-cli INFO stats | grep keyspace

# Ожидаемый вывод при хорошем кеше:
# keyspace_hits:95420
# keyspace_misses:512
# Hit rate = 95420 / (95420 + 512) = 99.4%
```

Низкий hit rate (< 70%) может указывать на:
- Слишком короткий TTL
- Частые инвалидации (много write-операций)
- Нехватку памяти и вытеснение ключей

---

## 6. Решение об архитектуре кеша

**Почему Cache-Aside, а не Read-Through:**  
userver не предоставляет встроенного Read-Through адаптера для Redis. Cache-Aside реализуется напрямую в слое хранилища (`UsersStorage`, `ItemsStorage`) и даёт полный контроль над логикой инвалидации.

**Почему Write-Through не используется для items:**  
Каталог хранится в MongoDB. Синхронная запись и в MongoDB, и в Redis при каждом `AddItem` увеличит latency write-операций. Вместо этого используется Write-Invalidate: при записи кеш удаляется, и следующий читатель заполняет его заново.

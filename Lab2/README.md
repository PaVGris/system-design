# Store 

REST API микросервисный магазин на C++ userver framework.

> [!NOTE]  
> В дальнейшем уберу папку Lab2 и сделаю репозиторий, как проект, но пока что так.
## Стек

- **Framework**: [userver](https://userver.tech/)
- **Язык**: C++20
- **Сборка**: CMake
- **Кодогенерация**: Chaotic (генерация DTO)
- **Тестирование**: pytest + userver testsuite
- **Контейнеризация**: Docker + Docker Compose

## ADR
|Index|Tags|Description|
|-----|----|-----------|
|[ADR-001](./decisions/ADR-001.md)|architecture, gateway, auth|API Gateway как единая точка входа|
|[ADR-002](./decisions/ADR-002.md)|auth, security|Chaotic для генерации DTO|
|[ADR-003](./decisions/ADR-003.md)| codegen, dto, chaotic|Session-based аутентификация|


## Архитектура
![image](./content/container.png)

### Сервисы

| Сервис | Порт | Описание |
|--------|------|----------|
| api-gateway | 8080 | Маршрутизация запросов, аутентификация |
| user-service | 8081 | Регистрация, логин, управление пользователями |
| item-service | 8082 | Каталог товаров |
| cart-service | 8083 | Корзина пользователя |

## Быстрый старт

### Требования

- Docker Desktop
- Docker Compose

### Запуск микросервисов
```bash
git clone <ссылка на репозиторий>
cd Lab2/store
docker-compose up --build
```
> [!IMPORTANT]  
> Такой запуск потребует много оперативной памяти, поэтому лучше использовать `Dev Container`.

![image](./content/docker.png)

Каждый микросервис запустить вот такой командой внутри определенной папки (например, user-service):
```bash
cd user-service
mkdir -p build && cd build
cmake ..
make -j4
```
### Проверка доступности сервера
```bash
curl http://localhost:8080/ping
```

## API

Полная документация: [`openapi.yaml`](./openapi.yaml)

Swagger UI не использовался, но использовался Postman.

### Аутентификация
```bash
# Регистрация
curl -X POST http://localhost:8080/users/register \
  -H "Content-Type: application/json" \
  -d '{"username": "pavel", "password": "123", "name": "Pavel", "role": "client"}'

# Логинимся и получаем токен
curl -X POST http://localhost:8080/users/login \
  -H "Content-Type: application/json" \
  -d '{"username": "pavel", "password": "123"}'
```

### Товары
```bash
# Получить список (нужен токен)
curl http://localhost:8080/items \
  -H "Authorization: Bearer <token>"

# Создать товар (только для seller)
curl -X POST http://localhost:8080/items \
  -H "Authorization: Bearer <token>" \
  -H "Content-Type: application/json" \
  -d '{"name": "Laptop", "price": 999.99, "quantity": 10}'
```

### Корзина
```bash
# Добавить товар
curl -X POST http://localhost:8080/cart \
  -H "Authorization: Bearer <token>" \
  -H "Content-Type: application/json" \
  -d '{"item_id": 1, "quantity": 2}'

# Посмотреть корзину
curl http://localhost:8080/cart \
  -H "Authorization: Bearer <token>"
```

## Роли

| Роль | Права |
|------|-------|
| client | Просмотр товаров, управление корзиной |
| seller | Всё выше + создание/изменение/удаление товаров |

## Структура проекта
```
content/                  # Картиночки
decisions/                # ADR
store/                    # Сам сервис
├── api-gateway/          # Маршрутизация и аутентификация
│   ├── configs/          # Конфигурация
│   ├── src/handlers/     # HTTP хендлеры
│   └── tests/            # Функциональные тесты
├── cart-service/         # Корзина
│   ├── configs/          # Конфигурация
│   ├── src/    
│   │   ├── handlers/     # HTTP хендлеры
│   │   └── storage/      # in-memory хранилище
│   ├── schemas/          # Схемы
│   └── tests/            # Функциональные тесты
├── item-service/         # Каталог товаров
│   ├── configs/          # Конфигурация
│   ├── src/    
│   │   ├── handlers/     # HTTP хендлеры
│   │   └── storage/      # in-memory хранилище
│   ├── schemas/          # Схемы
│   └── tests/            # Функциональные тесты
├── user-service/         # Пользователи и токены
│   ├── configs/          # Конфигурация
│   ├── src/    
│   │   ├── handlers/     # HTTP хендлеры
│   │   └── storage/      # in-memory хранилище
│   ├── schemas/          # Схемы
│   └── tests/            # Функциональные тесты
├── docker-compose.yaml
├── openapi.yaml          # API документация
```

## Тестирование

Каждый сервис имеет функциональные тесты на pytest через userver testsuite. Один из примеров тестирования:
```bash
cd user-service/build
ctest -V
```
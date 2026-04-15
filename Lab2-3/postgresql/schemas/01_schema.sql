-- Таблица пользователей
CREATE TABLE IF NOT EXISTS users (
    id SERIAL PRIMARY KEY,
    username TEXT UNIQUE NOT NULL,
    password TEXT NOT NULL,
    name TEXT NOT NULL,
    role TEXT NOT NULL CHECK (role IN ('client', 'seller')) 
);

-- Таблица товаров
CREATE TABLE IF NOT EXISTS items (
    id SERIAL PRIMARY KEY,
    name TEXT NOT NULL,
    description TEXT NOT NULL DEFAULT '',
    price INT NOT NULL CHECK (price >= 0), 
    quantity INT NOT NULL DEFAULT 0 CHECK (quantity >= 0) 
);

-- Таблица токенов
CREATE TABLE IF NOT EXISTS tokens (
    token TEXT PRIMARY KEY,
    user_id INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Таблица корзины
CREATE TABLE IF NOT EXISTS cart_items (
    user_id INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    item_id INTEGER NOT NULL REFERENCES items(id) ON DELETE CASCADE,
    quantity INTEGER NOT NULL DEFAULT 1 CHECK (quantity > 0),
    PRIMARY KEY (user_id, item_id)
);

-- Индексы для WHERE и JOIN (пункт 3 задания)
CREATE INDEX IF NOT EXISTS idx_users_name ON users(name text_pattern_ops);
CREATE INDEX IF NOT EXISTS idx_tokens_user_id ON tokens(user_id);
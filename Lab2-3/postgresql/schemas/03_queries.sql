-- 1. Создание нового пользователя 
INSERT INTO users (username, password, name, role) 
VALUES ('new_user', 'hashed_password', 'Aleksey Ivanov', 'client'); 

-- 2. Поиск пользователя по логину (точный поиск) 
SELECT id, username, name, role 
FROM users 
WHERE username = 'user1'; 

-- 3. Поиск пользователя по маске имени и фамилии 
-- Позволяет найти всех Иванов, Алексеев и т.д.
SELECT id, username, name, role 
FROM users 
WHERE name LIKE 'Ivan%';

-- 4. Создание товара (для роли seller)
INSERT INTO items (name, description, price, quantity) 
VALUES ('Smartphone', 'Latest model with OLED display', 5000000, 20); 

-- 5. Получение списка всех товаров 
SELECT id, name, description, price, quantity FROM items;

-- 6. Добавление товара в корзину для пользователя 
-- Если товар уже есть, обновляем количество (UPSERT)
INSERT INTO cart_items (user_id, item_id, quantity)
VALUES (2, 1, 1)
ON CONFLICT (user_id, item_id)
DO UPDATE SET quantity = cart_items.quantity + EXCLUDED.quantity;

-- 7. Получение корзины пользователя
SELECT i.name, i.price, c.quantity, (i.price * c.quantity) AS total_item_price
FROM cart_items c
JOIN items i ON c.item_id = i.id
WHERE c.user_id = 2; 

-- 8. Удаление товара из корзины 
DELETE FROM cart_items
WHERE user_id = 2 AND item_id = 1;

-- 9. Очистка корзины пользователя после заказа
DELETE FROM cart_items
WHERE user_id = 2;
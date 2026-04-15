INSERT INTO users (username, password, name, role) VALUES 
    ('admin', '123', 'Admin Adminov', 'seller'),
    ('user1', '123', 'Ivan Ivanov', 'client'),
    ('user2', '123', 'Ivan Petrov', 'client'),
    ('seller1', '123', 'Maria Sidorova', 'seller'),
    ('user3', '123', 'Anna Ivanova', 'client'),
    ('user4', '123', 'Sergey Volkov', 'client'),
    ('seller2', '123', 'Dmitry Kuznecov', 'seller'),
    ('user5', '123', 'Elena Popova', 'client'),
    ('user6', '123', 'Alexey Smirnov', 'client'),
    ('user7', '123', 'Olga Semenova', 'client')
ON CONFLICT (username) DO NOTHING;

INSERT INTO items (name, description, price, quantity) VALUES 
    ('Laptop', 'Powerful gaming laptop', 10000000, 5),
    ('Mouse', 'Wireless optical mouse', 150000, 50),
    ('Keyboard', 'Mechanical keyboard RGB', 500000, 30),
    ('Monitor', '27" 4K UHD Monitor', 3500000, 10),
    ('Headphones', 'Noise cancelling headphones', 800000, 25),
    ('Webcam', '1080p HD Webcam', 400000, 15),
    ('SSD 1TB', 'NVMe M.2 SSD', 1200000, 40),
    ('RAM 16GB', 'DDR4 3200MHz', 600000, 60),
    ('Power Bank', '20000mAh fast charging', 250000, 35),
    ('USB Cable', 'Type-C 2m braided', 50000, 100)
ON CONFLICT DO NOTHING;

INSERT INTO cart_items (user_id, item_id, quantity) VALUES 
    ((SELECT id FROM users WHERE username = 'user1'), (SELECT id FROM items WHERE name = 'Laptop'), 1),
    ((SELECT id FROM users WHERE username = 'user1'), (SELECT id FROM items WHERE name = 'Mouse'), 2),
    ((SELECT id FROM users WHERE username = 'user2'), (SELECT id FROM items WHERE name = 'Keyboard'), 1),
    ((SELECT id FROM users WHERE username = 'user3'), (SELECT id FROM items WHERE name = 'Monitor'), 1),
    ((SELECT id FROM users WHERE username = 'user4'), (SELECT id FROM items WHERE name = 'Headphones'), 1),
    ((SELECT id FROM users WHERE username = 'user5'), (SELECT id FROM items WHERE name = 'Webcam'), 1),
    ((SELECT id FROM users WHERE username = 'user6'), (SELECT id FROM items WHERE name = 'SSD 1TB'), 2),
    ((SELECT id FROM users WHERE username = 'user7'), (SELECT id FROM items WHERE name = 'RAM 16GB'), 4),
    ((SELECT id FROM users WHERE username = 'user1'), (SELECT id FROM items WHERE name = 'USB Cable'), 3),
    ((SELECT id FROM users WHERE username = 'user2'), (SELECT id FROM items WHERE name = 'Power Bank'), 1)
ON CONFLICT DO NOTHING;
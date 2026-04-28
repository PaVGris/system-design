import os
import psycopg2
from faker import Faker
import time
import uuid

# Настройки подключения
DB_SETTINGS = {
    "host": os.getenv("DB_HOST", "localhost"),
    "database": os.getenv("DB_NAME", "store_db"),
    "user": os.getenv("DB_USER", "user"),
    "password": os.getenv("DB_PASS", "pass")
}

def fill_users():
    fake = Faker()
    conn = psycopg2.connect(**DB_SETTINGS)
    cur = conn.cursor()

    # Найти максимальный id в таблице users
    cur.execute("SELECT COALESCE(MAX(id), 0) FROM users")
    max_id = cur.fetchone()[0]
    print(f"Текущий максимальный id: {max_id}")

    # Сколько нужно добавить до 1 000 000
    target_total = 1_000_000
    current_count = max_id
    need_to_add = target_total - current_count

    if need_to_add <= 0:
        print(f"В таблице уже {current_count} записей, больше или равно {target_total}. Нечего добавлять.")
        cur.close()
        conn.close()
        return

    print(f"Нужно добавить: {need_to_add} пользователей...")
    start_time = time.time()

    batch_size = 10000
    batches = (need_to_add + batch_size - 1) // batch_size

    for batch_num in range(batches):
        users = []
        current_batch_size = min(batch_size, need_to_add - batch_num * batch_size)
        
        for _ in range(current_batch_size):
            # Гарантированно уникальный username через UUID
            username = f"user_{uuid.uuid4().hex[:12]}"
            name = fake.name()
            if batch_num == 0 and _ < 10:
                name = f"Ivan {fake.last_name()}"
            
            users.append((username, 'password123', name, 'client'))
        
        cur.executemany(
            "INSERT INTO users (username, password, name, role) VALUES (%s, %s, %s, %s)",
            users
        )
        conn.commit()
        
        added = (batch_num + 1) * batch_size
        if added > need_to_add:
            added = need_to_add
        print(f"Загружено {added} / {need_to_add} строк...")

    end_time = time.time()
    print(f"Готово! Затрачено времени: {round(end_time - start_time, 2)} сек.")
    
    # Проверить итоговое количество
    cur.execute("SELECT COUNT(*) FROM users")
    final_count = cur.fetchone()[0]
    print(f"Всего пользователей в БД: {final_count}")
    
    cur.close()
    conn.close()

if __name__ == "__main__":
    fill_users()
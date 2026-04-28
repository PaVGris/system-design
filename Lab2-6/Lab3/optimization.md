# Оптимизации запросов

В ходе выполнения работы была проведена оптимизация производительности базы данных `store_db` при объеме данных в 1 000 000 записей (гененрировались пользователи с помощью файла [`fill_db.py`](../fill_db.py)).

## 1. Поиск пользователей по маске имени (`LIKE 'Ivan%'`)

Данный запрос используется для поиска пользователей по частичному совпадению имени.

### 1.1. До создания индекса (Последовательное сканирование)
Без индекса PostgreSQL выполняет параллельное сканирование всей таблицы (`Parallel Seq Scan`).

```console
user@99609f32476e:~/myservice$ psql -h db -U user -d store_db
Password for user user: 
psql (14.20 (Ubuntu 14.20-0ubuntu0.22.04.1), server 15.17)
WARNING: psql major version 14, server major version 15.
         Some psql features might not work.
Type "help" for help.

store_db=# DROP INDEX IF EXISTS idx_users_name;
DROP INDEX
store_db=# SELECT count(*) AS total_users FROM users;
 total_users 
-------------
      999980
(1 row)

```
**Запрос:**
```sql
EXPLAIN ANALYZE
SELECT id, username, name, role
FROM users
WHERE name LIKE 'Ivan%';
```
**План:**
```
                                                      QUERY PLAN                                                      
----------------------------------------------------------------------------------------------------------------------
 Gather  (cost=1000.00..16809.03 rows=98 width=42) (actual time=0.317..43.234 rows=178 loops=1)
   Workers Planned: 2
   Workers Launched: 2
   ->  Parallel Seq Scan on users  (cost=0.00..15799.23 rows=41 width=42) (actual time=5.486..37.446 rows=59 loops=3)
         Filter: (name ~~ 'Ivan%'::text)
         Rows Removed by Filter: 333267
 Planning Time: 0.366 ms
 Execution Time: 43.280 ms
(8 rows)
```
## 1.2. Создание индекса
Для оптимизации поиска по текстовой маске использован индекс с классом операторов text_pattern_ops, который позволяет эффективно использовать B-tree для оператора LIKE. Пробовал без этого оператора, и индекс был неэффективен, использовался обычный последовательный скан.
```sql
CREATE INDEX idx_users_name ON users(name text_pattern_ops);
```
## 1.3. После создания индекса
Планировщик переключился на использование индекса (Index Scan).

**План:**
```
                                                        QUERY PLAN                                                        
--------------------------------------------------------------------------------------------------------------------------
 Index Scan using idx_users_name on users  (cost=0.42..8.45 rows=98 width=42) (actual time=0.427..0.798 rows=178 loops=1)
   Index Cond: ((name ~>=~ 'Ivan'::text) AND (name ~<~ 'Ivao'::text))
   Filter: (name ~~ 'Ivan%'::text)
 Planning Time: 0.711 ms
 Execution Time: 0.831 ms

```
**Результат:** Время выполнения сократилось с 43.280 мс до 0.831 мс (примерно в 52 раза).


# 2. Поиск токена по user_id
Этот запрос важен для производительности, так как выполняется при каждой авторизации.

## 2.1. До создания индекса
Генерируем примерчики для индекса.
```console
store_db=# INSERT INTO tokens (token, user_id)
SELECT md5(id::text), id 
FROM users 
LIMIT 1000000;
INSERT 0 999980

```
Без индекса БД вынуждена перебирать все записи в таблице tokens.
**Запрос:**

```sql
EXPLAIN ANALYZE 
SELECT * FROM tokens WHERE user_id = 500000;
```
**План:**

```
                                               QUERY PLAN                                               
--------------------------------------------------------------------------------------------------------
 Seq Scan on tokens  (cost=0.00..18692.06 rows=1 width=44) (actual time=75.090..104.137 rows=1 loops=1)
   Filter: (user_id = 500000)
   Rows Removed by Filter: 999984
 Planning Time: 1.153 ms
 Execution Time: 104.151 ms
```
## 2.2. Создание индекса
Создаем индекс на внешний ключ user_id.

```sql
CREATE INDEX idx_tokens_user_id ON tokens(user_id);
```
## 2.3. После создания индекса (Bitmap Index Scan)
Поиск теперь выполняется мгновенно через индекс.

**План:**

```
                                                          QUERY PLAN                                                           
-------------------------------------------------------------------------------------------------------------------------------
 Bitmap Heap Scan on tokens  (cost=95.17..11373.33 rows=5000 width=44) (actual time=0.057..0.057 rows=1 loops=1)
   Recheck Cond: (user_id = 500000)
   Heap Blocks: exact=1
   ->  Bitmap Index Scan on idx_tokens_user_id  (cost=0.00..93.92 rows=5000 width=0) (actual time=0.040..0.040 rows=1 loops=1)
         Index Cond: (user_id = 500000)
 Planning Time: 0.255 ms
 Execution Time: 0.140 ms
```
**Результат:** Время выполнения сократилось со 104.151 мс до 0.140 мс (~ в 740 раз).

# Выводы
-- Индексы на внешние ключи обязательны для предотвращения деградации производительности при росте данных.

-- Специальные классы операторов (как text_pattern_ops) необходимы для оптимизации специфических поисковых запросов (LIKE).
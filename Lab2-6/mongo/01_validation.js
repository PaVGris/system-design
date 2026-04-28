db.runCommand({
  collMod: "items",
  validator: {
    $jsonSchema: {
      bsonType: "object",
      required: ["name", "price", "quantity"],
      additionalProperties: true,
      properties: {
        _id: {
          bsonType: "objectId"
        },
        name: {
          bsonType: "string",
          minLength: 2,
          maxLength: 200,
          description: "Название товара — обязательное строковое поле, от 2 до 200 символов"
        },
        description: {
          bsonType: "string",
          maxLength: 2000,
          description: "Описание товара — необязательное, до 2000 символов"
        },
        price: {
          bsonType: "double",
          minimum: 0.01,
          description: "Цена товара — обязательное число, больше 0"
        },
        quantity: {
          bsonType: "int",
          minimum: 0,
          description: "Количество на складе — обязательное целое число, не меньше 0"
        }
      }
    }
  },
  validationLevel: "strict",
  validationAction: "error"
});

print("Валидация для коллекции 'items' установлена.");

db.runCommand({
  collMod: "carts",
  validator: {
    $jsonSchema: {
      bsonType: "object",
      required: ["user_id", "items"],
      additionalProperties: true,
      properties: {
        _id: {
          bsonType: "objectId"
        },
        user_id: {
          bsonType: "int",
          minimum: 1,
          description: "ID пользователя из PostgreSQL — обязательное целое число"
        },
        items: {
          bsonType: "array",
          description: "Список позиций корзины — обязательный массив",
          items: {
            bsonType: "object",
            required: ["item_id", "quantity"],
            properties: {
              item_id: {
                bsonType: "string",
                minLength: 24,
                maxLength: 24,
                description: "ObjectId товара в виде строки — 24 hex символа"
              },
              quantity: {
                bsonType: "int",
                minimum: 1,
                description: "Количество — целое число, минимум 1"
              }
            }
          }
        }
      }
    }
  },
  validationLevel: "strict",
  validationAction: "error"
});

print("Валидация для коллекции 'carts' установлена.");

print("\n--- Тестируем невалидные данные ---");

print("\nТест 1: вставка товара без поля 'name' (должна быть ошибка)");
try {
  db.items.insertOne({
    description: "Нет названия",
    price: 100.0,
    quantity: 5
  });
  print("ПРОВАЛ: документ вставлен без ошибки");
} catch (e) {
  print("УСПЕХ: ошибка валидации —", e.message);
}

print("\nТест 2: вставка товара с отрицательной ценой (должна быть ошибка)");
try {
  db.items.insertOne({
    name: "Плохой товар",
    description: "Отрицательная цена",
    price: -100.0,
    quantity: 5
  });
  print("ПРОВАЛ: документ вставлен без ошибки");
} catch (e) {
  print("УСПЕХ: ошибка валидации —", e.message);
}

print("\nТест 3: вставка товара с дробным quantity (должна быть ошибка)");
try {
  db.items.insertOne({
    name: "Ещё товар",
    price: 500.0,
    quantity: 2.5     // должно быть int
  });
  print("ПРОВАЛ: документ вставлен без ошибки");
} catch (e) {
  print("УСПЕХ: ошибка валидации —", e.message);
}

print("\nТест 4: вставка корзины с quantity=0 в позиции (должна быть ошибка)");
try {
  db.carts.insertOne({
    user_id: 999,
    items: [{ item_id: "507f1f77bcf86cd799439011", quantity: 0 }]
  });
  print("ПРОВАЛ: документ вставлен без ошибки");
} catch (e) {
  print("УСПЕХ: ошибка валидации —", e.message);
}

print("\nТест 5: вставка корзины с коротким item_id (должна быть ошибка)");
try {
  db.carts.insertOne({
    user_id: 998,
    items: [{ item_id: "12345", quantity: 1 }]  // не 24 символа
  });
  print("ПРОВАЛ: документ вставлен без ошибки");
} catch (e) {
  print("УСПЕХ: ошибка валидации —", e.message);
}

print("\nТест 6: вставка валидного товара (должна пройти успешно)");
try {
  db.items.insertOne({
    name: "Валидный товар",
    description: "Всё в порядке",
    price: NumberDouble(299.99),
    quantity: NumberInt(10)
  });
  print("УСПЕХ: валидный документ вставлен");
} catch (e) {
  print("ПРОВАЛ: ошибка —", e.message);
}

db.items.deleteOne({ name: "Валидный товар" });

print("\nВалидация: ОК.");

// 1. Вставка одного товара
db.items.insertOne({
  name: "Новый товар",
  description: "Описание нового товара",
  price: 1999.99,
  quantity: 10
});

// 2. Вставка нескольких товаров сразу
db.items.insertMany([
  { name: "Товар A", description: "Описание A", price: 500.00, quantity: 5 },
  { name: "Товар B", description: "Описание B", price: 750.00, quantity: 3 }
]);

// 3. Создание корзины для нового пользователя (upsert)
db.carts.updateOne(
  { user_id: 99 },
  { $setOnInsert: { user_id: 99, items: [] } },
  { upsert: true }
);

// 4. Получить все товары
db.items.find({});

// 5. Найти товар по точному названию ($eq)
db.items.findOne({ name: { $eq: "Ноутбук Lenovo IdeaPad 3" } });

// 6. Найти товары дороже 10000 рублей ($gt)
db.items.find({ price: { $gt: 10000 } });

// 7. Найти товары дешевле 5000 рублей ($lt)
db.items.find({ price: { $lt: 5000 } });

// 8. Найти товары в диапазоне цен от 5000 до 30000 ($gte, $lte)
db.items.find({ price: { $gte: 5000, $lte: 30000 } });

// 9. Найти товары с количеством не равным нулю ($ne)
db.items.find({ quantity: { $ne: 0 } });

// 10. Найти товары по списку названий ($in)
db.items.find({
  name: { $in: ["Клавиатура Logitech MX Keys", "Мышь Logitech MX Master 3"] }
});

// 11. Найти товары НЕ из списка ($nin)
db.items.find({
  name: { $nin: ["Товар A", "Товар B"] }
});

// 12. Найти дорогие товары с достаточным количеством ($and)
db.items.find({
  $and: [
    { price: { $gt: 20000 } },
    { quantity: { $gt: 10 } }
  ]
});

// 13. Найти дешёвые товары ИЛИ товары с большим запасом ($or)
db.items.find({
  $or: [
    { price: { $lt: 4000 } },
    { quantity: { $gt: 100 } }
  ]
});

// 14. Получить корзину пользователя
db.carts.findOne({ user_id: 1 });

// 15. Найти все корзины содержащие конкретный товар
db.carts.find({ "items.item_id": "507f1f77bcf86cd799439011" });

// 16. Найти корзины с более чем 2 позициями
db.carts.find({ $where: "this.items.length > 2" });

// 17. Поиск с проекцией — только name и price
db.items.find({}, { name: 1, price: 1, _id: 0 });

// 18. Поиск с сортировкой по цене (ascending)
db.items.find({}).sort({ price: 1 });

// 19. Поиск с пагинацией
db.items.find({}).sort({ _id: 1 }).skip(0).limit(5);

// 20. Обновить цену конкретного товара ($set)
db.items.updateOne(
  { name: "Товар A" },
  { $set: { price: 599.99 } }
);

// 21. Уменьшить количество товара при покупке ($inc)
db.items.updateOne(
  { name: "Товар A" },
  { $inc: { quantity: -1 } }
);

// 22. Добавить товар в корзину ($push)
db.carts.updateOne(
  { user_id: 1 },
  { $push: { items: { item_id: "507f1f77bcf86cd799439099", quantity: 1 } } }
);

// 23. Добавить товар если его нет в корзине ($addToSet с $each)
const userId = 1;
const newItemId = "507f1f77bcf86cd799439099";
const existsInCart = db.carts.findOne({
  user_id: userId,
  "items.item_id": newItemId
});
if (!existsInCart) {
  db.carts.updateOne(
    { user_id: userId },
    { $push: { items: { item_id: newItemId, quantity: 1 } } }
  );
}

// 24. Увеличить quantity существующего товара в корзине ($inc на элемент массива)
db.carts.updateOne(
  { user_id: 1, "items.item_id": "507f1f77bcf86cd799439099" },
  { $inc: { "items.$.quantity": 1 } }
);

// 25. Массово обновить — поднять цены всех товаров на 10% ($mul)
db.items.updateMany(
  {},
  { $mul: { price: 1.1 } }
);

// 26. Очистить корзину ($set пустой массив)
db.carts.updateOne(
  { user_id: 4 },
  { $set: { items: [] } }
);


// 27. Удалить товар из корзины ($pull)
db.carts.updateOne(
  { user_id: 1 },
  { $pull: { items: { item_id: "507f1f77bcf86cd799439099" } } }
);

// 28. Удалить конкретный товар из каталога
db.items.deleteOne({ name: "Товар A" });

// 29. Удалить все товары с нулевым количеством
db.items.deleteMany({ quantity: { $eq: 0 } });

// 30. Удалить корзину пользователя
db.carts.deleteOne({ user_id: 99 });

// 31. Подсчитать общее количество товаров в каждой корзине
db.carts.aggregate([
  { $match: { "items.0": { $exists: true } } },   // только непустые корзины
  { $project: {
    user_id: 1,
    total_items: { $size: "$items" },
    total_quantity: { $sum: "$items.quantity" }
  }},
  { $sort: { total_quantity: -1 } }
]);

// 32. Топ-3 самых дорогих товара
db.items.aggregate([
  { $sort: { price: -1 } },
  { $limit: 3 },
  { $project: { name: 1, price: 1, _id: 0 } }
]);

// 33. Средняя цена всех товаров
db.items.aggregate([
  { $group: {
    _id: null,
    avg_price: { $avg: "$price" },
    min_price: { $min: "$price" },
    max_price: { $max: "$price" },
    total_items: { $sum: 1 }
  }}
]);

print("Все запросы выполнены.");

// ============================================================
// Коллекция: items
// ============================================================

db.items.drop();

const items = db.items.insertMany([
  {
    name: "Ноутбук Lenovo IdeaPad 3",
    description: "15.6 дюймов, Intel Core i5-1235U, 8GB RAM, 512GB SSD",
    price: 54999.99,
    quantity: 42
  },
  {
    name: "Смартфон Samsung Galaxy A54",
    description: "6.4 дюймов, 8GB RAM, 256GB, камера 50MP",
    price: 29999.00,
    quantity: 118
  },
  {
    name: "Наушники Sony WH-1000XM5",
    description: "Беспроводные, активное шумоподавление, 30 часов работы",
    price: 24990.00,
    quantity: 35
  },
  {
    name: "Клавиатура Logitech MX Keys",
    description: "Беспроводная, подсветка, мультиустройственная",
    price: 8990.00,
    quantity: 67
  },
  {
    name: "Монитор Dell U2722D",
    description: "27 дюймов, 4K IPS, USB-C 90W, sRGB 100%",
    price: 44900.00,
    quantity: 15
  },
  {
    name: "Мышь Logitech MX Master 3",
    description: "Беспроводная, эргономичная, 4000 DPI",
    price: 6490.00,
    quantity: 93
  },
  {
    name: "SSD Samsung 970 EVO Plus 1TB",
    description: "M.2 NVMe, скорость чтения 3500 MB/s",
    price: 7990.00,
    quantity: 54
  },
  {
    name: "Веб-камера Logitech C920",
    description: "Full HD 1080p, автофокус, встроенный микрофон",
    price: 5490.00,
    quantity: 28
  },
  {
    name: "Планшет Apple iPad 10",
    description: "10.9 дюймов, A14 Bionic, 64GB, Wi-Fi",
    price: 49990.00,
    quantity: 22
  },
  {
    name: "Зарядная станция Anker 736",
    description: "100W GaN, 6 портов, совместима с MacBook",
    price: 4990.00,
    quantity: 110
  },
  {
    name: "Игровая мышь Razer DeathAdder V3",
    description: "Проводная, 30000 DPI, оптический сенсор Focus Pro",
    price: 5990.00,
    quantity: 41
  },
  {
    name: "USB-хаб Ugreen 9-in-1",
    description: "HDMI 4K, USB 3.0 x3, SD/microSD, USB-C PD 100W",
    price: 3490.00,
    quantity: 76
  }
]);

print("Вставлено товаров:", db.items.countDocuments());

// Сохраняем id первых двух товаров для корзин
const itemIds = db.items.find({}, { _id: 1 }).limit(5).toArray().map(d => d._id.toString());
print("ID первых 5 товаров:", itemIds);

// ============================================================
// Коллекция: carts
// ============================================================

db.carts.drop();

db.carts.insertMany([
  {
    user_id: 1,
    items: [
      { item_id: itemIds[0], quantity: 1 },
      { item_id: itemIds[1], quantity: 2 }
    ]
  },
  {
    user_id: 2,
    items: [
      { item_id: itemIds[2], quantity: 1 }
    ]
  },
  {
    user_id: 3,
    items: [
      { item_id: itemIds[0], quantity: 3 },
      { item_id: itemIds[3], quantity: 1 },
      { item_id: itemIds[4], quantity: 1 }
    ]
  },
  {
    user_id: 4,
    items: []
  },
  {
    user_id: 5,
    items: [
      { item_id: itemIds[1], quantity: 1 },
      { item_id: itemIds[2], quantity: 2 }
    ]
  },
  {
    user_id: 6,
    items: [
      { item_id: itemIds[4], quantity: 1 }
    ]
  },
  {
    user_id: 7,
    items: [
      { item_id: itemIds[0], quantity: 2 },
      { item_id: itemIds[1], quantity: 1 }
    ]
  },
  {
    user_id: 8,
    items: [
      { item_id: itemIds[3], quantity: 4 }
    ]
  },
  {
    user_id: 9,
    items: [
      { item_id: itemIds[2], quantity: 1 },
      { item_id: itemIds[4], quantity: 1 }
    ]
  },
  {
    user_id: 10,
    items: [
      { item_id: itemIds[0], quantity: 1 }
    ]
  }
]);

print("Вставлено корзин:", db.carts.countDocuments());

db.items.createIndex({ name: 1 });
db.carts.createIndex({ user_id: 1 }, { unique: true });
db.carts.createIndex({ "items.item_id": 1 });

print("Индексы созданы.");
print("Инициализация: ОК.");

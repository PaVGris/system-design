workspace "Магазин" "Азон" {

    !identifiers hierarchical

    model {
        regular_user = person "Client" {
            tag "Person"
        }

        seller = person "Seller" {
            tag "Person"
        }
        
        store = softwareSystem "Software System" {
            psql = container "Database for Users and Items" {
                description "Хранит данные пользователей и товаров"
                technology "PostgreSQL 18.3"
                tag "db"
            }

            redis = container "Database for Carts" {
                description "Хранит корзины пользователей"
                technology "Redis 8.4.2"
                tag "db"
            }

            user = container "User Service" {
                description "Управление профилями и аутентификация"
                technology "Userver"
                -> psql "Add a new User or Find a user by login or name"
            }

            cart = container "Cart Service" {
                description "Управление составом корзин"
                technology "Userver Framework"
                -> redis "Get cart / Add Item to user's cart "
                -> user "Get user's id"
            }
            
            item = container "Item Service" {
                description "Каталог и управление товарами"
                technology "Userver Framework"
                -> psql "Add a new Item or Get list of all "
            }

            api_gateway = container "API Gateway" {
                description "Перенаправление запросов по сервисам"
                technology "Userver Framework"
                -> user "Routes user-related requests" "HTTPS"
                -> item "Routes item-related requests" "HTTPS"
                -> cart "Routes cart-related requests" "HTTPS"
            }

            web_browser = container "Web Browser" {
                description "Способ взаимодействия с системой"
                tag "Web"
                -> api_gateway "Makes API calls to" "HTTPS"
            }
        
            
            cart -> item "Get Item's id"

            regular_user -> web_browser "Sing up / Get cart / Get Items / Add Item to Cart"
            seller -> web_browser "Find Client / Add new Item"
        }
    
    }
    views {
        systemContext store {
            include *
            autolayout lr
        }

        container store {
            include *
            autolayout tb
        }

        dynamic store "Add_to_Cart_Flow" "Сценарий добавления товара в корзину" {
        regular_user -> store.web_browser "Нажимает 'Добавить в корзину'"
        store.web_browser -> store.api_gateway "POST /cart/add {itemId}"
        store.api_gateway -> store.cart "Перенаправляет запрос"
        
        store.cart -> store.user "Запрашивает данные пользователя"
        store.user -> store.psql "Проверяет сессию/пользователя"
        
        store.cart -> store.item "Запрашивает информацию о товаре"
        store.item -> store.psql "Проверяет наличие товара"
        
        store.cart -> store.redis "Сохраняет товар в корзину пользователя"
        
        store.cart -> store.api_gateway "Возвращает успех (200 OK)"
        store.api_gateway -> store.web_browser "Отображает товар в корзине"
        
        autolayout lr
    }

        styles 
            element "db" {
                shape Cylinder
                background #06488A
                color #ffffff
            }

            element "Web" {
                shape WebBrowser
            }

            element "Person" {
                background #FA9D24
                shape person
            }

            element "Software System" {
                strokeWidth 3 
                background #1167BD
                color #ffffff
            }
        }
    }
}
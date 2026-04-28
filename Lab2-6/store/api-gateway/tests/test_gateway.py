async def test_ping(service_client):
    response = await service_client.get('/ping')
    assert response.status == 200


async def test_register(service_client, mockserver):
    @mockserver.json_handler('/user-service/users/register')
    def mock_register(request):
        return {'id': 1, 'username': 'pavel'}

    response = await service_client.post(
        '/users/register',
        json={
            'username': 'pavel',
            'password': '123',
            'name': 'Pavel',
            'role': 'client'
        }
    )
    assert response.status == 200


async def test_login(service_client, mockserver):
    @mockserver.json_handler('/user-service/users/login')
    def mock_login(request):
        return {'token': 'test-token-123'}

    response = await service_client.post(
        '/users/login',
        json={'username': 'pavel', 'password': '123'}
    )
    assert response.status == 200
    assert 'token' in response.json()


async def test_unauthorized(service_client, mockserver):
    @mockserver.json_handler('/user-service/users/validate')
    def mock_validate(request):
        return mockserver.make_response(status=401)

    response = await service_client.get('/items')
    assert response.status == 401


async def test_get_items_as_client(service_client, mockserver):
    @mockserver.json_handler('/user-service/users/validate')
    def mock_validate(request):
        return {'user_id': 1, 'valid': True, 'role': 'client'}

    @mockserver.json_handler('/item-service/items')
    def mock_items(request):
        return [{'id': 1, 'name': 'laptop', 'price': 999.99, 'quantity': 5}]

    response = await service_client.get(
        '/items',
        headers={'Authorization': 'Bearer test-token'}
    )
    assert response.status == 200


async def test_create_item_forbidden_for_client(service_client, mockserver):
    @mockserver.json_handler('/user-service/users/validate')
    def mock_validate(request):
        return {'user_id': 1, 'valid': True, 'role': 'client'}

    response = await service_client.post(
        '/items',
        json={'name': 'laptop', 'price': 999.99, 'quantity': 5},
        headers={'Authorization': 'Bearer test-token'}
    )
    assert response.status == 403


async def test_create_item_allowed_for_seller(service_client, mockserver):
    @mockserver.json_handler('/user-service/users/validate')
    def mock_validate(request):
        return {'user_id': 1, 'valid': True, 'role': 'seller'}

    @mockserver.json_handler('/item-service/items')
    def mock_create_item(request):
        return {'id': 1}

    response = await service_client.post(
        '/items',
        json={'name': 'laptop', 'price': 999.99, 'quantity': 5},
        headers={'Authorization': 'Bearer test-token'}
    )
    assert response.status == 200
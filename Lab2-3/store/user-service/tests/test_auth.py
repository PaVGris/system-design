async def test_ping(service_client):
    response = await service_client.get('/ping')
    assert response.status == 200


async def test_register(service_client):
    response = await service_client.post(
        '/users/register',
        json={
            'username': 'testuser',
            'password': '123',
            'name': 'Test User',
            'role': 'client'
        }
    )
    assert response.status == 201
    data = response.json()
    assert 'id' in data
    assert data['username'] == 'testuser'


async def test_register_duplicate(service_client):
    await service_client.post(
        '/users/register',
        json={
            'username': 'duplicate',
            'password': '123',
            'name': 'Test',
            'role': 'client'
        }
    )
    response = await service_client.post(
        '/users/register',
        json={
            'username': 'duplicate',
            'password': '123',
            'name': 'Test',
            'role': 'client'
        }
    )
    assert response.status == 400


async def test_login(service_client):
    await service_client.post(
        '/users/register',
        json={
            'username': 'loginuser',
            'password': '123',
            'name': 'Login User',
            'role': 'client'
        }
    )
    response = await service_client.post(
        '/users/login',
        json={'username': 'loginuser', 'password': '123'}
    )
    assert response.status == 200
    data = response.json()
    assert 'token' in data


async def test_login_wrong_password(service_client):
    response = await service_client.post(
        '/users/login',
        json={'username': 'loginuser', 'password': 'wrong'}
    )
    assert response.status == 401


async def test_get_user_unauthorized(service_client):
    response = await service_client.get('/users/1')
    assert response.status == 401


async def test_get_user(service_client):
    await service_client.post(
        '/users/register',
        json={
            'username': 'getuser',
            'password': '123',
            'name': 'Get User',
            'role': 'client'
        }
    )
    login = await service_client.post(
        '/users/login',
        json={'username': 'getuser', 'password': '123'}
    )
    token = login.json()['token']

    response = await service_client.get(
        '/users/1',
        headers={'Authorization': f'Bearer {token}'}
    )
    assert response.status == 200
    data = response.json()
    assert 'username' in data
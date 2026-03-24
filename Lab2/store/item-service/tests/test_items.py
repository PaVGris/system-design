async def test_ping(service_client):
    response = await service_client.get('/ping')
    assert response.status == 200


async def test_get_items_empty(service_client):
    response = await service_client.get('/items')
    assert response.status == 200
    assert response.json() == []


async def test_create_item(service_client):
    response = await service_client.post(
        '/items',
        json={
            'name': 'laptop',
            'description': 'good laptop',
            'price': 999.99,
            'quantity': 10
        }
    )
    assert response.status == 201
    data = response.json()
    assert 'id' in data


async def test_get_item(service_client):
    create = await service_client.post(
        '/items',
        json={'name': 'phone', 'price': 499.99, 'quantity': 5}
    )
    item_id = create.json()['id']

    response = await service_client.get(f'/items/{item_id}')
    assert response.status == 200
    data = response.json()
    assert data['name'] == 'phone'


async def test_update_item(service_client):
    create = await service_client.post(
        '/items',
        json={'name': 'tablet', 'price': 299.99, 'quantity': 3}
    )
    item_id = create.json()['id']

    response = await service_client.put(
        f'/items/{item_id}',
        json={'price': 199.99}
    )
    assert response.status == 200


async def test_delete_item(service_client):
    create = await service_client.post(
        '/items',
        json={'name': 'headphones', 'price': 99.99, 'quantity': 20}
    )
    item_id = create.json()['id']

    response = await service_client.delete(f'/items/{item_id}')
    assert response.status == 204

    response = await service_client.get(f'/items/{item_id}')
    assert response.status == 404


async def test_get_item_not_found(service_client):
    response = await service_client.get('/items/99999')
    assert response.status == 404
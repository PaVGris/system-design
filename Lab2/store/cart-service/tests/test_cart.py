async def test_ping(service_client):
    response = await service_client.get('/ping')
    assert response.status == 200


async def test_get_cart_unauthorized(service_client):
    response = await service_client.get('/cart')
    assert response.status == 401


async def test_add_to_cart(service_client):
    response = await service_client.post(
        '/cart',
        json={'item_id': 1, 'quantity': 2},
        headers={'X-User-Id': '1'}
    )
    assert response.status == 200


async def test_get_cart(service_client):
    await service_client.post(
        '/cart',
        json={'item_id': 1, 'quantity': 2},
        headers={'X-User-Id': '1'}
    )

    response = await service_client.get(
        '/cart',
        headers={'X-User-Id': '1'}
    )
    assert response.status == 200
    data = response.json()
    assert 'items' in data
    assert len(data['items']) == 1


async def test_remove_from_cart(service_client):
    await service_client.post(
        '/cart',
        json={'item_id': 5, 'quantity': 1},
        headers={'X-User-Id': '1'}
    )

    response = await service_client.delete(
        '/cart/5',
        headers={'X-User-Id': '1'}
    )
    assert response.status == 204
FAKE_ITEM_ID = '507f1f77bcf86cd799439011'

async def test_ping(service_client):
    response = await service_client.get('/ping')
    assert response.status == 200


async def test_get_cart_unauthorized(service_client):
    response = await service_client.get('/cart')
    assert response.status == 401


async def test_add_to_cart(service_client, mock_item_service):
    response = await service_client.post(
        '/cart',
        json={'item_id':  FAKE_ITEM_ID, 'quantity': 2},
        headers={'X-User-Id': '1'}
    )
    assert response.status == 200
    assert mock_item_service.times_called > 0

async def test_get_cart(service_client, mock_item_service):
    await service_client.post(
        '/cart',
        json={'item_id':  FAKE_ITEM_ID, 'quantity': 2},
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
    assert data['items'][0]['item_id'] == FAKE_ITEM_ID

async def test_remove_from_cart(service_client, mock_item_service):
    await service_client.post(
        '/cart',
        json={'item_id': FAKE_ITEM_ID, 'quantity': 1},
        headers={'X-User-Id': '1'}
    )

    response = await service_client.delete(
        '/cart/' + FAKE_ITEM_ID,
        headers={'X-User-Id': '1'}
    )
    assert response.status in (200, 204)
import pytest

pytest_plugins = ['pytest_userver.plugins.core']

USERVER_CONFIG_HOOKS = ['patch_cart_service_config']

@pytest.fixture(scope='session')
def patch_cart_service_config(mockserver_info):
    def do_patch(config_yaml, config_vars):
        components = config_yaml['components_manager']['components']
        
        if 'handler-add-to-cart' in components:
            components['handler-add-to-cart']['item-service-url'] = mockserver_info.url('item-service')
            
    return do_patch

@pytest.fixture(name='mock_item_service')
def _mock_item_service(mockserver):
    @mockserver.json_handler('/item-service/items/', prefix=True)
    def _handler(request):
        item_id = request.path.split('/')[-1]
        return {
            'id': item_id,
            'name': 'Test Item',
            'price': 100,
            'quantity': 1000
        }
    return _handler

@pytest.fixture(scope='session')
def service_env():
    return {
        'MONGO_URI': 'mongodb://mongo-db:27017/store_db',
    }

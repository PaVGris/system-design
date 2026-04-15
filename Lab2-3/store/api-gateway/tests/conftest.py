import pytest

pytest_plugins = ['pytest_userver.plugins.core']

USERVER_CONFIG_HOOKS = ['userver_config_services']

@pytest.fixture(scope='session')
def userver_config_services(mockserver_info):
    def do_patch(config_yaml, config_vars):
        config_vars['user-service-url'] = mockserver_info.url('user-service')
        config_vars['item-service-url'] = mockserver_info.url('item-service')
        config_vars['cart-service-url'] = mockserver_info.url('cart-service')
    return do_patch
import pytest

pytest_plugins = ['pytest_userver.plugins.core']

@pytest.fixture(scope='session')
def service_env():
    return {
        'MONGO_URI': 'mongodb://mongo-db:27017/store_db',
    }
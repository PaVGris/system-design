FROM ghcr.io/userver-framework/ubuntu-22.04-userver:latest

COPY . /app
WORKDIR /app

RUN mkdir -p build && cd build && cmake .. && make -j4

CMD ["./build/api-gateway", "--config", "configs/config.yaml", "--config_vars", "configs/config_vars.yaml"]
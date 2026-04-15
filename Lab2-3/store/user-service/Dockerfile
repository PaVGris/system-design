FROM ghcr.io/userver-framework/ubuntu-22.04-userver:latest

COPY . /app
WORKDIR /app

RUN mkdir -p build && cd build && cmake .. && make -j4

CMD ["./build/user-service", "--config", "configs/config.yaml"]
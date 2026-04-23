FROM rockylinux:9 AS builder

RUN curl https://sh.rustup.rs -sSf | sh -s -- -y
ENV PATH="/root/.cargo/bin:${PATH}"

WORKDIR /app

RUN cargo build --release

FROM rockylinux:9

WORKDIR /app

CMD ["/app/my_http_server"]


FROM ubuntu:22.04

# Установка зависимостей для сборки kubsh и тестов
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    g++ \
    libreadline-dev \
    libfuse3-dev \
    pkg-config \
    fuse3 \
    sudo \
    python3 \
    python3-pip \
    adduser \
    && rm -rf /var/lib/apt/lists/*

# Копируем код kubsh
WORKDIR /app
COPY . .

# Сборка kubsh
RUN mkdir build && cd build && cmake .. && make -j$(nproc)

# Устанавливаем kubsh в PATH
RUN cp build/kubsh /usr/local/bin/kubsh
RUN chmod +x /usr/local/bin/kubsh

# Копируем тесты из opt_from_container
COPY opt_from_container /opt

# Устанавливаем pytest
RUN pip install pytest

# Создаём точку монтирования для VFS
RUN mkdir -p /root/users

# Запуск тестов
CMD ["python3", "/opt/test_basic.py", "/opt/test_vfs.py"]

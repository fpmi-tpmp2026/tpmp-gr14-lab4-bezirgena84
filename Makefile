# Конфигурация компилятора
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iincludes -g
LDFLAGS = -lsqlite3 -lssl -lcrypto

# Директории
SRC_DIR = src
INC_DIR = includes
BUILD_DIR = build
BIN_DIR = bin

# Исходные файлы
SRCS = $(SRC_DIR)/main.c \
       $(SRC_DIR)/auth.c \
       $(SRC_DIR)/database.c \
       $(SRC_DIR)/country.c \
       $(SRC_DIR)/region.c

# Объектные файлы
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Имя исполняемого файла
TARGET = $(BIN_DIR)/country_app

# Цели по умолчанию
all: directories $(TARGET)

# Создание директорий
directories:
	@mkdir -p $(BUILD_DIR) $(BIN_DIR)

# Линковка
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "✓ Сборка завершена: $(TARGET)"

# Компиляция исходных файлов
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Очистка
clean:
	rm -rf $(BUILD_DIR)/*.o $(TARGET)
	@echo "✓ Очистка завершена"

# Полная пересборка
rebuild: clean all

# Запуск приложения
run: all
	./$(TARGET)

# Тестирование
test: all
	@echo "Запуск тестов..."
	./$(TARGET)

# Установка зависимостей (для Linux)
install-deps:
	@echo "Установка зависимостей..."
	@sudo apt-get update && sudo apt-get install -y libsqlite3-dev libssl-dev

.PHONY: all directories clean rebuild run test install-deps

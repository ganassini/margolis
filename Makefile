CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_GNU_SOURCE
LDFLAGS = -lpthread -lrt
TARGET = margolis
SOURCE = margolis.c

.PHONY: all clean run debug

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -O2 -o $(TARGET) $(SOURCE) $(LDFLAGS)

debug: $(SOURCE)
	$(CC) $(CFLAGS) -g -DDEBUG -o $(TARGET)_debug $(SOURCE) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

run-debug: debug
	gdb ./$(TARGET)_debug

clean:
	rm -f $(TARGET) $(TARGET)_debug

# Diferentes configurações para experimentos
run-low-resources: $(TARGET)
	@echo "Executando com recursos limitados (modificar código: 1 pista, 2 portões)"
	./$(TARGET)

run-high-traffic: $(TARGET)
	@echo "Executando com tráfego intenso (modificar código: intervalos menores)"
	./$(TARGET)

install-deps:
	@echo "Instalando dependências no Ubuntu/Debian:"
	sudo apt-get update
	sudo apt-get install gcc libc6-dev build-essential gdb

help:
	@echo "Comandos disponíveis:"
	@echo "  make          - Compila o programa"
	@echo "  make debug    - Compila com símbolos de debug"
	@echo "  make run      - Executa o programa"
	@echo "  make clean    - Remove arquivos compilados"
	@echo "  make help     - Mostra esta ajuda"
	@echo ""
	@echo "Para experimentos, modifique as constantes no código:"
	@echo "  NUM_PISTAS, NUM_PORTOES, MAX_TORRE_OPERACOES"

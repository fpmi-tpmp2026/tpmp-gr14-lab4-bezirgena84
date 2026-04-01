CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iincludes -g
LDFLAGS = -lsqlite3 -lssl -lcrypto

SRCDIR = src
INCDIR = includes
BUILDDIR = build
BINDIR = bin

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SOURCES))
TARGET = $(BINDIR)/country_manager

all: directories $(TARGET)

directories:
	mkdir -p $(BINDIR) $(BUILDDIR)

$(TARGET): $(OBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BINDIR)/* $(BUILDDIR)/*

.PHONY: all clean directories
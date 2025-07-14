# VGA Font Editor Makefile for MinGW-w64

CC = gcc
WINDRES = windres
CFLAGS = -Wall -Wextra -std=c99 -O2
LDFLAGS = -mwindows
LIBS = -lgdi32 -luser32 -lkernel32 -lcomdlg32

TARGET = vga-font-editor.exe
SOURCES = main.c font_manager.c controls.c
OBJECTS = $(SOURCES:.c=.o)
RESOURCE_OBJ = resource.o

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS) $(RESOURCE_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c vga_font_editor.h
	$(CC) $(CFLAGS) -c $< -o $@

$(RESOURCE_OBJ): resource.rc vga_font_editor.h
	$(WINDRES) -i $< -o $@

clean:
	del /Q *.o $(TARGET) 2>nul || true

install: $(TARGET)
	copy $(TARGET) "C:\Program Files\VGA Font Editor\" 2>nul || echo "Run as administrator to install to Program Files"

# Development targets
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

release: CFLAGS += -DNDEBUG
release: clean $(TARGET)

# Help target
help:
	@echo Available targets:
	@echo   all       - Build the application (default)
	@echo   clean     - Remove object files and executable
	@echo   debug     - Build with debug symbols
	@echo   release   - Build optimized release version
	@echo   install   - Install to Program Files (requires admin)
	@echo   help      - Show this help message

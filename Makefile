# Makefile: building server/client with SDL2
# ==== OS-detektering ====
ifeq ($(OS), Windows_NT)
	OS := Windows_NT
else 
	OS := $(shell uname -s 2>/dev/null)
endif
# $(info === Detekterat OS: $(OS))

ifeq ($(OS), Darwin)
# --- macOS Settings ---
    CC = clang
    CFLAGS = -fsanitize=address -fsanitize=undefined -g -Wall -Wextra \
             -I/opt/homebrew/include/SDL2 \
             -I/opt/homebrew/include/SDL2_image \
             -I/opt/homebrew/include/SDL2_ttf \
             -I/opt/homebrew/include/SDL2_mixer \
             -I/opt/homebrew/include/SDL2_net
    LDFLAGS = -fsanitize=address \
              -L/opt/homebrew/lib \
              -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_net \
              -lpthread
    REMOV = rm -rf build/*.o $(EXEC)
    EXEC = build/main
    RUN = ./
    PREFORM = ioio
else ifeq ($(OS), Windows_NT)
# --- Windows (MinGW/MSYS) Settings ---
	CC = gcc
	INCLUDE = C:/msys64/mingw64/include/SDL2
	CFLAGS = -g -Wall -Wextra -I$(INCLUDE)
	LDFLAGS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf \
			  -lSDL2_mixer -lSDL2_net -lws2_32
	REMOV = del /Q $(BUILDDIR)\*.o & if exist $(CLIENT_EXEC) del /Q $(CLIENT_EXEC) & if exist $(SERVER_EXEC) del /Q $(SERVER_EXEC)
	EXEC = build/main.exe
	RUN = ./
	PREFORM =
else ifeq ($(OS), Linux)
# --- Linux (Fedora/Ubuntu) Settings with clang and sanitizers ---
	CC = clang
	CFLAGS = -fsanitize=address -fsanitize=undefined -g -Wall -Wextra `sdl2-config --cflags`
	LDFLAGS = -fsanitize=address `sdl2-config --libs` -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_net -lpthread
	REMOV = rm -rf build/*.o $(EXEC)
	EXEC = build/main
	RUN = ./
	PREFORM =
endif

# ==== Vanliga variabler ====
TARGET = $(EXEC)
SRCDIR = source
BUILDDIR = build
OBJ = $(BUILDDIR)/main.o $(BUILDDIR)/init.o 

# Default Goal
all: $(BUILDDIR) $(TARGET)

# Create build folder if it doesn't exist
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Client build rule
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET) $(LDFLAGS)

# Individuella objektfiler
$(BUILDDIR)/main.o: $(SRCDIR)/main.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/init.o: $(SRCDIR)/init.c
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	$(REMOV)

run:
	$(RUN)$(TARGET)




CC = gcc
CFLAGS = -Wall -Wextra -std=c99 \
         -Isrc -Isrc/shared -Isrc/backend -Isrc/frontend -Isrc/controller

# ─── Platform Detection ───
# Windows (MinGW / MSYS2)
LDFLAGS = -lfreeglut -lopengl32 -lglu32 -lm
# Linux — uncomment below and comment above:
# LDFLAGS = -lGL -lGLU -lglut -lm
# macOS — uncomment below:
# LDFLAGS = -framework OpenGL -framework GLUT -lm

BACKEND_SRC = src/backend/seir.c \
              src/backend/timer.c \
              src/backend/economy.c \
              src/backend/mental_health.c \
              src/backend/hospital.c \
              src/backend/decisions.c \
              src/backend/end_conditions.c \
              src/backend/debriefing.c \
              src/backend/simulation.c

FRONTEND_SRC = src/frontend/renderer.c \
               src/frontend/camera.c \
               src/frontend/city_renderer.c \
               src/frontend/entity_renderer.c \
               src/frontend/hud.c \
               src/frontend/menu.c \
               src/frontend/input.c

CONTROLLER_SRC = src/controller/controller.c \
                 src/controller/world_gen.c

SRC = src/main.c $(BACKEND_SRC) $(FRONTEND_SRC) $(CONTROLLER_SRC) src/shared/texture_loader.c
OBJ = $(SRC:.c=.o)
TARGET = pandemic_sim

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Backend-only unit tests (no OpenGL needed)
test: test/test_backend.c $(BACKEND_SRC)
	$(CC) $(CFLAGS) test/test_backend.c $(BACKEND_SRC) -o test_backend -lm
	./test_backend

clean:
	del /Q src\backend\*.o src\frontend\*.o src\controller\*.o src\*.o $(TARGET).exe test_backend.exe 2>nul || true

.PHONY: all clean test

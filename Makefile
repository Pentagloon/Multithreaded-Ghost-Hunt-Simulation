CC      = gcc
CFLAGS  = -Wall -Wextra -g -std=c11 -fsanitize=thread
LDFLAGS = -fsanitize=thread

TARGET  = p5

RUNNER  = runner

SOURCES = main.c \
          ghost.c \
          helpers.c \
          house.c \
          hunter.c \
          room.c

OBJECTS = $(SOURCES:.c=.o)

RUNNER_SOURCES = runner.c
RUNNER_OBJECTS = $(RUNNER_SOURCES:.c=.o)

HEADERS = defs.h \
          ghost.h \
          helpers.h \
          house.h \
          hunter.h \
          room.h

all: $(TARGET) $(RUNNER)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS)

$(RUNNER): $(RUNNER_OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(RUNNER_OBJECTS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean run run_runner

run: $(TARGET)
	./$(TARGET)

run_runner: $(RUNNER)
	./$(RUNNER)

clean:
	rm -f $(OBJECTS) $(RUNNER_OBJECTS) $(TARGET) $(RUNNER) log_*.csv

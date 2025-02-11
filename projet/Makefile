CC = gcc
CFLAGS = -Wall -Wextra -Werror
TARGET = my_shell

all: $(TARGET)

$(TARGET): main.o functions.o
	$(CC) $(CFLAGS) -o $(TARGET) main.o functions.o

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

functions.o: functions.c
	$(CC) $(CFLAGS) -c functions.c

clean:
	rm -f $(TARGET) *.o


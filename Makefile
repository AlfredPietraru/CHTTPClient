CC = gcc 
CFLAGS = -g -Wall -Wextra
TARGET = client

all: $(TARGET)

run:
	./$(TARGET) 

$(TARGET): $(TARGET).c 
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c parson.c

help: 
	$(CC) $(CFLAGS) -o help parson.c

clean:
	rm $(TARGET)


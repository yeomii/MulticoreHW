TARGET=mat_mul
OBJS=mat_mul.o timers.o

CC=gcc
CFLAGS=-g -O3 -Wall -fopenmp
LDFLAGS= 

all: $(TARGET)

$(TARGET):$(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $@

clean:
	rm -rf $(TARGET) $(OBJS) task*

run: $(TARGET)
	thorq --add ./$(TARGET) -t 32

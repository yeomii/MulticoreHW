TARGET=mat_mul
OBJS=mat_mul.o timers.o util.o opencl_util.o

CC=gcc
CFLAGS=-g -O3 -Wall -std=c99
LDFLAGS= -lOpenCL

all: $(TARGET)

$(TARGET):$(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

clean:
	rm -rf $(TARGET) $(OBJS) task*

run: $(TARGET)
	thorq --add --device gpu ./$(TARGET) -n 10000 -g 10000 -w 16

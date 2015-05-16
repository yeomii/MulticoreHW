TARGET=vec_add_host
OBJS=vec_add_host.o

CC=gcc
CFLAGS= -std=c99
LDFLAGS= -lOpenCL

all: $(TARGET)

$(TARGET):$(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

clean:
	rm -rf $(TARGET) $(OBJS) task*

run: $(TARGET)
	thorq --add --device gpu ./$(TARGET) 

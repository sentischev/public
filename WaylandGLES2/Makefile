
CC=gcc
CFLAGS=-Og -I.
LDFLAGS=-lwayland-client -lwayland-server -lwayland-egl -lEGL -lGLESv2

DEPS=WaylandGLES2.h
OBJS=WaylandGLES2.o

%.o: %.c $(DEPS)
	$(CC) -o $@ -c $< $(CFLAGS)

WaylandGLES2: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf *.o WaylandGLES2

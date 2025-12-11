CC = gcc
CFLAGS = -fPIC -Wall -Wextra -I../../include $(shell pkg-config --cflags gobject-2.0)
LDFLAGS = -shared -L../../lib -ljabra $(shell pkg-config --libs gobject-2.0)

TARGET = libjabbar-gobject.so
GIR = JabBar-1.0.gir
TYPELIB = JabBar-1.0.typelib

SRCS = src/jabbar-manager.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET) $(TYPELIB)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


$(GIR): $(TARGET)
	ln -sf ../../lib/libjabra.so.1.12.2.0 libjabra.so.1
	LD_LIBRARY_PATH=.:../../lib:$(LD_LIBRARY_PATH) g-ir-scanner src/jabbar-manager.c src/jabbar-manager.h \
		--library=jabbar-gobject \
		--extra-library=jabra \
		--library-path=. \
		--library-path=../../lib \
		--include=GObject-2.0 \
		--namespace=JabBar \
		--symbol-prefix=jabbar \
		--nsversion=1.0 \
		--output=$@ \
		--c-include="jabbar-manager.h" \
		--cflags-begin -I../../include $(shell pkg-config --cflags gobject-2.0) --cflags-end

$(TYPELIB): $(GIR)
	g-ir-compiler $< --output=$@

clean:
	rm -f $(OBJS) $(TARGET) $(GIR) $(TYPELIB)

all:
	$(MAKE) -C src/wrapper
	cp src/wrapper/libjabbar-gobject.so src/applet/
	cp src/wrapper/JabBar-1.0.typelib src/applet/
	cp lib/libjabra.so.1.12.2.0 src/applet/libjabra.so
	ln -sf libjabra.so src/applet/libjabra.so.1
	patchelf --set-rpath '$$ORIGIN' src/applet/libjabbar-gobject.so

clean:
	$(MAKE) -C src/wrapper clean
	rm -f src/applet/*.so* src/applet/*.typelib

.PHONY: all clean

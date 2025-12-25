CC=gcc
CFLAGS=-Wall -Wextra -std=c11
TARGET=kubsh

.PHONY: all run clean deb

all: $(TARGET)

$(TARGET): kubsh.c
	$(CC) $(CFLAGS) -o $(TARGET) kubsh.c

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) *.o

deb: $(TARGET)
	mkdir -p deb/DEBIAN
	mkdir -p deb/usr/local/bin
	cp $(TARGET) deb/usr/local/bin/
	echo "Package: kubsh" > deb/DEBIAN/control
	echo "Version: 1.0" >> deb/DEBIAN/control
	echo "Section: utils" >> deb/DEBIAN/control
	echo "Priority: optional" >> deb/DEBIAN/control
	echo "Architecture: amd64" >> deb/DEBIAN/control
	echo "Maintainer: YOURNAME <youremail@example.com>" >> deb/DEBIAN/control
	echo "Description: Simple Linux shell for GNU/Linux course" >> deb/DEBIAN/control
	dpkg-deb --build deb
	mv deb.deb kubsh_1.0.deb
	rm -rf deb

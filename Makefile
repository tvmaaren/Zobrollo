PREFIX ?= /usr
DATADIR ?= ${PREFIX}/share/zobrollo
LICENSEDIR ?= /usr/share/licenses
BINDIR ?= ${PREFIX}/bin
PLATFORM ?= linux

OBJ =src/config.o src/display.o src/drawframe.o src/drawtrack.o src/ghost.o src/gui.o src/kart.o src/main.o src/misc.o src/race.o src/record.o  src/connect_server.o src/networking.o src/input_boxes.o 
libs=-lagui -lagui_allegro5 -lallegro -lallegro_primitives -lm -lallegro_font -lallegro_ttf -lallegro_image  


CFLAGS= -g

ifeq ($(PLATFORM),  linux)
	CC=gcc
	CXX=g++
	executable=zobrollo
endif

ifeq ($(PLATFORM),  windows)
	CC=x86_64-w64-mingw32-gcc
	CXX=x86_64-w64-mingw32-g++
	libs += -lws2_32 #winsock is only for windows
	executable=zobrollo.exe
endif

default: zobrollo

zobrollo: $(OBJ)
	$(CXX) $(CFLAGS) $(OBJ) $(libs) -o $(executable)

src/main.o: src/main.c src/config.o src/misc.o src/file_paths.h src/race.o src/record.o src/ghost.o src/gui.o src/input_boxes.o
src/input_boxes.o: src/input_boxes.cpp
src/race.o: src/race.c src/drawtrack.o src/kart.o src/config.o src/file_paths.h src/misc.o src/record.o src/drawframe.o
src/record.o:src/record.c src/file_paths.h src/config.o src/misc.o src/gui.o src/ghost.o
src/ghost.o:src/ghost.c src/config.o src/drawtrack.o src/misc.o src/drawframe.o
src/drawframe.o: src/drawframe.c src/config.o src/kart.o src/drawtrack.o src/misc.o
src/drawtrack.o:  src/drawtrack.c src/misc.o
src/connect_server.o: src/connect_server.c
src/gui.o:src/gui.c src/config.o 
src/config.o: src/config.c src/file_paths.h
src/kart.o: src/kart.c
src/misc.o: src/misc.c
src/display: src/display.c
src/file_paths.h:
src/networking.o: src/networking.c


clean:
	rm src/*.o $(executable)

install: default
	install -Dm755 zobrollo ${DESTDIR}${BINDIR}/zobrollo
	install -Dm644 config.cfg ${DESTDIR}${DATADIR}/config.cfg
	install -Dm644 full\ heart.png ${DESTDIR}${DATADIR}/full\ heart.png
	install -Dm644 half\ heart.png ${DESTDIR}${DATADIR}/half\ heart.png
	install -Dm644 tracks/example  ${DESTDIR}${DATADIR}/tracks/example
	install -Dm644 tracks/Long\ Straight  ${DESTDIR}${DATADIR}/tracks/Long\ straight
	install -Dm644 tracks/triangle  ${DESTDIR}${DATADIR}/tracks/triangle	
	install -Dm644 LICENSE ${DESTDIR}${LICENSEDIR}/zobrollo/LICENSE

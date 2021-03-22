PREFIX ?= /usr
DATADIR ?= ${PREFIX}/share/zobrollo
LICENSEDIR ?= /usr/share/licenses
BINDIR ?= ${PREFIX}/bin

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
libs=-lallegro -lallegro_primitives -lm -lallegro_font -lallegro_ttf -lallegro_image

default: $(OBJ)
	gcc $(OBJ) $(libs) -o zobrollo

src\main.o: src\main.c src\config.o src\misc.o src\file_paths.h src\race.o src\record.o src\ghost.o src\gui.o
	gcc -c -g src\main.c -o src\main.o
src\race.o: src\drawtrack.o src\kart.o src\config.o src\file_paths.h src\race.c src\misc.o src\record.o src\drawframe.o
	gcc -c -g src\race.c -o src\race.o
src\record.o: src\file_paths.h src\config.o src\record.c src\misc.o src\gui.o src\ghost.o
	gcc -c -g src\record.c -o src\record.o
src\ghost.o:  src\config.o src\drawtrack.o src\misc.o src\ghost.c src\drawframe.o
	gcc -c -g src\ghost.c -o src\ghost.o
src\drawframe.o: src\drawframe.c src\config.o src\kart.o src\drawtrack.o src\misc.o
	gcc -c -g src\drawframe.c -o src\drawframe.o
src\drawtrack.o:  src\drawtrack.c src\misc.o
	gcc -c -g src\drawtrack.c -o src\drawtrack.o
src\gui.o:  src\config.o src\gui.c
	gcc -c -g src\gui.c -o src\gui.o
src\config.o: src\config.c src\file_paths.h
	gcc -c -g src\config.c -o src\config.o
src\kart.o: src\kart.c
	gcc -c -g src\kart.c -o src\kart.o
src\misc.o: src\misc.c
	gcc -c -g src\misc.c -o src\misc.o
src\file_paths.h:


clean:
	rm src\*.o zobrollo

install: default
	install -Dm755 zobrollo ${DESTDIR}${BINDIR}/zobrollo
	install -Dm644 config.cfg ${DESTDIR}${DATADIR}/config.cfg
	install -Dm644 full\ heart.png ${DESTDIR}${DATADIR}/full\ heart.png
	install -Dm644 half\ heart.png ${DESTDIR}${DATADIR}/half\ heart.png
	install -Dm644 tracks/example  ${DESTDIR}${DATADIR}/tracks/example
	install -Dm644 tracks/Long\ Straight  ${DESTDIR}${DATADIR}/tracks/Long\ straight
	install -Dm644 tracks/triangle  ${DESTDIR}${DATADIR}/tracks/triangle	
	install -Dm644 LICENSE ${DESTDIR}${LICENSEDIR}/zobrollo/LICENSE

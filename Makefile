PREFIX ?= /usr
DATADIR ?= ${PREFIX}/share/zobrollo
LICENSEDIR ?= /usr/share/licenses
BINDIR ?= ${PREFIX}/bin


default:  main

show: showtrack

main: main.c config.o race.o misc.o record.o file_paths.h
	cc  -g main.c record.o config.o race.o drawtrack.o kart.o misc.o -lallegro -lallegro_primitives -lm -lallegro_font -lallegro_ttf -lallegro_image -o zobrollo


race.o: race.c race.h drawtrack.o kart.o file_paths.h
	cc -c -g race.c -o race.o


drawtrack.o: drawtrack.c drawtrack.h misc.o
	cc -c -g drawtrack.c -o drawtrack.o

kart.o: kart.c kart.h
	cc -c -g kart.c -o kart.o


config.o: config.c config.h file_paths.h
	cc -c -g config.c -o config.o

record.o: record.h record.c misc.o
	cc -c -g record.c -o record.o

misc.o: misc.c misc.h
	cc -c -g misc.c -o misc.o


file_paths.h:

clean:
	rm *.o zobrollo

install: main
	install -Dm755 zobrollo ${DESTDIR}${BINDIR}/zobrollo
	install -Dm644 config.cfg ${DESTDIR}${DATADIR}/config.cfg
	install -Dm644 full\ heart.png ${DESTDIR}${DATADIR}/full\ heart.png
	install -Dm644 half\ heart.png ${DESTDIR}${DATADIR}/half\ heart.png
	install -Dm644 tracks/example  ${DESTDIR}${DATADIR}/tracks/example
	install -Dm644 tracks/Long\ Straight  ${DESTDIR}${DATADIR}/tracks/Long\ straight
	install -Dm644 LICENSE ${DESTDIR}${LICENSEDIR}/zobrollo/LICENSE

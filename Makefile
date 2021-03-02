PREFIX ?= /usr
DATADIR ?= ${PREFIX}/share/zobrollo
LICENSEDIR ?= /usr/share/licenses
BINDIR ?= ${PREFIX}/bin

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
libs=-lallegro -lallegro_primitives -lm -lallegro_font -lallegro_ttf -lallegro_image

default: $(OBJ)
	x86_64-w64-mingw32-gcc  $(OBJ) $(libs) -o zobrollo

%.o : %.c
	x86_64-w64-mingw32-gcc -o $@ -c $<

clean:
	rm src/*.o zobrollo

install: main
	install -Dm755 zobrollo ${DESTDIR}${BINDIR}/zobrollo
	install -Dm644 config.cfg ${DESTDIR}${DATADIR}/config.cfg
	install -Dm644 full\ heart.png ${DESTDIR}${DATADIR}/full\ heart.png
	install -Dm644 half\ heart.png ${DESTDIR}${DATADIR}/half\ heart.png
	install -Dm644 tracks/example  ${DESTDIR}${DATADIR}/tracks/example
	install -Dm644 tracks/Long\ Straight  ${DESTDIR}${DATADIR}/tracks/Long\ straight
	install -Dm644 LICENSE ${DESTDIR}${LICENSEDIR}/zobrollo/LICENSE

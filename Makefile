default:  main

show: showtrack

main: main.c config.o race.o misc.o
	cc  -g main.c config.o race.o drawtrack.o kart.o misc.o -lallegro -lallegro_primitives -lm -lallegro_font -lallegro_ttf -lallegro_audio -lallegro_image -o zobrollo


race.o: race.c race.h drawtrack.o kart.o
	cc -c -g race.c -o race.o -lallegro -lallegro_primitives -lm -lallegro_font -lallegro_ttf -lallegro_audio -lallegro_image


drawtrack.o: drawtrack.c drawtrack.h misc.o
	cc -c -g drawtrack.c -o drawtrack.o

kart.o: kart.c kart.h
	cc -c -g kart.c -o kart.o


config.o: config.c config.h
	cc -c -g config.c -o config.o

misc.o: misc.c misc.h
	cc -c -g misc.c -o misc.o

clean:
	rm *.o zobrollo

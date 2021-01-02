default:  race

show: showtrack


race: race.c drawtrack.o kart.o
	cc -g race.c drawtrack.o kart.o -o race -lallegro -lallegro_primitives -lm -lallegro_font -lallegro_ttf -lallegro_audio -lallegro_image


drawtrack.o: drawtrack.c
	cc -c -g drawtrack.c -o drawtrack.o

kart.o: kart.c
	cc -c -g kart.c -o kart.o

showtrack: showtrack.c drawtrack.o
	cc -g showtrack.c drawtrack.o -o showtrack -lallegro -lallegro_primitives -lm -lallegro_font -lallegro_ttf -lallegro_audio -lallegro_image

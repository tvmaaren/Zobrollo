cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lallegro -lallegro_font -lallegro_ttf -lallegro_image -lallegro_primitives -lm")

# set the project name
project(Zobrollo)

# add the executable
add_executable(zobrollo src/config.c src/drawframe.c src/drawtrack.c src/ghost.c src/gui.c src/kart.c src/main.c src/misc.c src/race.c src/record.c)


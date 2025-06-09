#!/bin/sh

OUT="yard"
FLAGS="-Wall -Wextra -Werror -Wpedantic -std=c23"
DEF="-DDEV"
SRC=$(find ./src/ -type f -name "*.c" | tr '\n' ' ')
LIBS="
vendor/glad/glad.o
vendor/stb_image/stb_image.o
-lglfw
-lm
"
INCS="
-I./vendor/glad/include
-I./vendor/stb_image
-I./include
"

rm -f ./$OUT
cc $FLAGS $DEF $LIBS $INCS $SRC -o $OUT
./$OUT

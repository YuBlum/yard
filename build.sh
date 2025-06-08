#!/bin/sh

OUT="yard"
FLAGS="-Wall -Wextra -Werror -Wpedantic -std=c11"
DEF="-DDEV"
SRC=$(find ./src/ -type f -name "*.c" | tr '\n' ' ')
LIBS="
vendor/glad/glad.o
-lglfw
-lm
"
INCS="
-I./vendor/glad/include
-I./include
"

rm -f ./$OUT
cc $FLAGS $DEF $LIBS $INCS $SRC -o $OUT
./$OUT

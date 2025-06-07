#!/bin/sh

OUT="jarpg"
SRC=""
LIBS="
vendor/glad/glad.o -I./vendor/glad/include
-lglfw
"

for i in ./src/*.c; do
  [ -f "$i" ] || break
  SRC="$SRC $i"
done

rm -f ./$OUT
cc -Wall -Wextra -Werror -Wpedantic -std=c99 $SRC $LIBS -o $OUT
./$OUT

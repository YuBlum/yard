#!/bin/sh

OUT="yard"
SRC=""
LIBS="
vendor/glad/glad.o -I./vendor/glad/include
-lglfw
-lm
"
DEF="-DDEV"

for i in ./src/*.c; do
  [ -f "$i" ] || break
  SRC="$SRC $i"
done

rm -f ./$OUT
cc -Wall -Wextra -Werror -Wpedantic -std=c11 $DEF $SRC $LIBS -o $OUT
./$OUT

#!/bin/bash

CC="clang"
INC="-Iexternal"
CFLAGS="-pipe -O2 -Wall -Wextra -Wpedantic -Werror -Wno-comment -Wno-unused-function -std=gnu11 ${INC}"
LFLAGS="-lSDL2 -lm"

mkdir -p bin

if ! $(test -f bin/external_impl.o); then
  ${CC} -pipe -O3 -c src/external_impl.c -o bin/external_impl.o
fi

${CC} ${CFLAGS} -c src/unity.c -o bin/unity.o

${CC} bin/*.o ${LFLAGS} -o bin/mop.bin


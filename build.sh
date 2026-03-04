set -xe

CC="clang"
CFLAGS="`pkg-config --cflags --libs x11 x11-xcb gl` -lm ${1}"
TARGET="coomer"
SRCS="coomer.c platform_linux.c platform_win32.c globals.c input.c render.c cd_clock.c"

${CC} ${SRCS} -o ${TARGET} ${CFLAGS}

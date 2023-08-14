CFLAGS		+= -I./sources/common
CPPFLAGS	+= -I./sources/common

VPATH += sources/common

C_SOURCES += sources/common/ring_buffer.c
C_SOURCES += sources/common/cmd_line.c
C_SOURCES += sources/common/xprintf.c




#
# Makefile for the malloc lab driver
#
CC = gcc
CFLAGS = -Wall -Wextra -Werror -O3 -DDRIVER -std=gnu99 -Wno-unused-function -Wno-unused-parameter -lm

# For debug
# CFLAGS = -Wall -Wextra -Werror -g -DDRIVER -std=gnu99 -Wno-unused-function -Wno-unused-parameter -lm
# For profile
# CFLAGS = -Wall -Wextra -Werror -O3 -pg -DDRIVER -std=gnu99 -Wno-unused-function -Wno-unused-parameter -lm -fno-inline

OBJS = mdriver.o mm.o memlib.o fsecs.o fcyc.o clock.o ftimer.o 

all: mdriver

mdriver: $(OBJS)
	$(CC) $(CFLAGS) -o mdriver $(OBJS)

mdriver.o: mdriver.c fsecs.h fcyc.h clock.h memlib.h config.h mm.h
memlib.o: memlib.c memlib.h
mm.o: mm.c mm.h memlib.h
fsecs.o: fsecs.c fsecs.h config.h
fcyc.o: fcyc.c fcyc.h
ftimer.o: ftimer.c ftimer.h config.h
clock.o: clock.c clock.h

clean:
	rm -f *~ *.o mdriver




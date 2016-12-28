CXX = gcc
CFLAGS = -Wall -Werror
CFLAGS += -g
#CFLAGS += -O3
LIBS    =

CORE = ips2pco
OBJS = bips.o blib.o util.o
HFILES = Makefile
CFILES = bips.c blib.c util.c

all: $(CORE)

$(CORE): $(OBJS) $(CFILES) $(HFILES)
	$(CXX) $(CFLAGS) -o $(CORE) $(OBJS) $(LIBS)

clean:
	rm -f $(CORE)
	rm -f *.o

%.o: %.c $(HFILES)
	$(CXX) -c $(CFLAGS) -DOSSD_TARGET=OSSD_IF_LINUX $< -o $@

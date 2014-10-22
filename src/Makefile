CC=gcc
CFLAGS=-c -Wall -Wextra -pedantic -std=c99 -g
LDFLAGS= -lpthread
SOURCES=main.c splitstream.c pipe.c perf.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=parfqz

all: $(SOURCES) $(EXECUTABLE)

clean:
	rm -f ${OBJECTS}
	rm -f ${EXECUTABLE}

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

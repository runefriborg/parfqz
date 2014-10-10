CC=gcc
CFLAGS=-c -Wall
LDFLAGS=
SOURCES=main.c splitstream.c
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

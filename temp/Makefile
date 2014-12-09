CC=g++
CFLAGS= -std=c++11 `pkg-config opencv --cflags` -g
LDFLAGS=`pkg-config opencv --libs` -g
SOURCES=temp.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=ass2

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

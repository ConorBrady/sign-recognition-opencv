CC=g++
CFLAGS= -std=c++11 \
		`pkg-config opencv --cflags` \
		-g -DGEN
LDFLAGS=`pkg-config opencv --libs` \
		-g -lboost_filesystem-mt -lboost_system
SOURCES= \
		main.cpp \
		known_sign.cpp \
		region_of_interest.cpp \
		search_image.cpp \
		back_projection_packer.cpp

OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=vision_assignment

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

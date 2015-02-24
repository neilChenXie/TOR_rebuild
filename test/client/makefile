EXE=client
FILES=client.cpp
OBJECTS=$(FILES:.cpp=.o)
CC=g++
CFLAGS=-g -c -Wall
$(EXE):$(OBJECTS)
	$(CC) $(OBJECTS) -o $@
%.o:%.cpp
	$(CC) $(CFLAGS) $< -o $@
clean:
	rm $(EXE)
	rm $(OBJECTS)

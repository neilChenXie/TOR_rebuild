EXE=proxy
FILES=main.cpp proxy.cpp 
OBJECT=$(FILES:.cpp=.o)
CFLAGS=-g -c -Wall
CC=g++
$(EXE):$(OBJECT)
	$(CC) $(OBJECT) -o $@
%.o:%.cpp
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(OBJECT)
	rm $(EXE)

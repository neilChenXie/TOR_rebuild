FILE=main.cpp router.cpp
OBJ=$(FILE:.cpp=.o)
CFLAGS=-g -c -Wall
CC=g++
EXE=router
$(EXE):$(OBJ)
	$(CC) $(OBJ) -o $@
%.o:%.cpp
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(EXE)
	rm $(OBJ)

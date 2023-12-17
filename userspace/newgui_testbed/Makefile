OBJ = $(patsubst %.c, %.o, $(wildcard *.c))
EXE = newgui_testbed.exe

CFLAGS += -I../../sgfx -I../../kernel/include -I../
LDFLAGS += -L../../sgfx -lsgfx

all: $(EXE)

$(EXE): $(OBJ)
	$(LD) -o $(EXE) $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f *.o $(EXE)
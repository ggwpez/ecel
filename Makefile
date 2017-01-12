HDR:=defines.h ent.h message.h io.h
SRC:=main.c ent.c message.c io.c

all: ecel

ecel: $(HDR) $(SRC)
	$(CC) $(SRC) -o ecel

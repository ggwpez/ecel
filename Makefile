HDR:=defines.h ent.h message.h
SRC:=main.c ent.c message.c

all: ecel

ecel: $(HDR) $(SRC)
	$(CC) $(SRC) -o ecel

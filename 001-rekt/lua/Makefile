CFLAGS=-I./ext/lua/src -c -fPIC
LLIB=--whole-archive ./ext/lua/src/liblua.a -lm --no-whole-archive

default: all

all: librekt.so

lua:
	cd ext/lua && $(MAKE) posix

lsrc.h: src/main.lua lua
	$(shell ext/lua/src/luac -o lsrc src/main.lua)
	$(shell xxd -i lsrc lsrc.h)
	$(shell rm -f lsrc)

main.o: main.c lsrc.h lua
	$(CC) $(CFLAGS) -o $@ $<

librekt.so: main.o 
	$(LD) $(LLIB) $^ -shared -o $@ 

clean:
	rm -f librekt.so *.o *.out lsrc.h

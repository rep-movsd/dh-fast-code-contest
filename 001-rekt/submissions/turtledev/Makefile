default: all

all: main.lua
	@echo build success

main.lua: src/*.lua
	$(shell luajit scripts/pack.lua src/main.lua)
	$(shell mv main.bundle.lua main.lua)


clean:
	rm -f main.lua

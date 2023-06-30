cc = g++
include_dirs = vendor
build_flags = -pthread
create_deps = -MMD
include_flags = $(foreach dir,$(include_dirs),-I$(dir))

src = $(wildcard src/*.cpp)
objects = $(patsubst src/%.cpp,obj/%.o,$(src))
deps = $(wildcard dep/*.d)

include $(deps)

bin/server: $(objects) vendor/picohttpparser/picohttpparser.o
	$(cc) $(build_flags) -o $@ $^

obj/%.o: src/%.cpp
	$(cc) $(include_flags) $(create_deps) -o dep/$(*F) $^;\
	$(cc) -c $(include_flags) -o $@ $^

vendor/%.o: vendor/%.c
	$(cc) -c -o $@ $^

.PHONY: clean
clean:
	- rm $(objects) $(deps) bin/server
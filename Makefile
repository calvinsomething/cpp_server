cc = g++
include_dirs = vendor
build_flags = -pthread
create_deps = -M -MF dep/$(*F).d
include_flags = $(foreach dir,$(include_dirs),-I$(dir))

binary = bin/server

src = $(wildcard src/*.cpp)
vendor_c = $(wildcard vendor/*/*.c)

objects = $(patsubst src/%.cpp,obj/%.o,$(src)) $(patsubst vendor/%.c,vendor/%.o,$(vendor_c))
deps = $(wildcard dep/*.d)

include $(deps)

$(binary): $(objects)
	$(cc) $(build_flags) -o $@ $^

obj/%.o: src/%.cpp
	$(cc) -E $(include_flags) $(create_deps) $^ &&\
	$(cc) -c $(include_flags) -o $@ $^

vendor/%.o: vendor/%.c
	$(cc) -c -o $@ $^

.PHONY: clean
clean:
	- rm $(wildcard obj/*.o) $(deps) $(binary)
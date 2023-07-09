cc = g++
include_dirs = vendor
build_flags = -pthread
create_deps = -MM -MF dep/$(*F).d -MQ $@
include_flags = $(foreach dir,$(include_dirs),-I$(dir))

binary = bin/server

src = $(wildcard src/*.cpp)
deps = $(wildcard dep/*.d)
vendor_c = $(wildcard vendor/*/*.c)
includes = $(wildcard dep/*.d)

objects = $(patsubst src/%.cpp,obj/%.o,$(src)) $(patsubst vendor/%.c,vendor/%.o,$(vendor_c))

$(binary): $(objects)
	$(cc) $(build_flags) -o $@ $^

obj/%.o: src/%.cpp
	$(cc) -E $(include_flags) $(create_deps) $^ &&\
	$(cc) -c $(include_flags) -o $@ $^

vendor/%.o: vendor/%.c
	$(cc) -c -o $@ $^

include $(includes) # not working... dependency list from .d files get used in obj/%.o rule (caused by repeated target??)

.PHONY: clean
clean:
	- rm $(wildcard obj/*.o) $(deps) $(binary)
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

objects = $(src:src/%.cpp=obj/%.o) $(vendor_c:.c=.o)

$(binary): $(objects)
	$(cc) $(build_flags) -o $@ $^

obj/%.o: src/%.cpp
	$(cc) -E $(include_flags) $(create_deps) $< &&\
	$(cc) -c $(include_flags) -o $@ $<

vendor/%.o: vendor/%.c
	$(cc) -c -o $@ $^

.PHONY: clean
clean:
	- rm $(wildcard obj/*.o) $(deps) $(binary)

include $(includes)
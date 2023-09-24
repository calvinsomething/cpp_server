cc = g++
include_dirs = vendor
build_flags = -pthread
create_deps = -MM -fpch-deps -fpch-preprocess -MF dep/$(*F).d -MQ $@
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
	$(cc) -E $(create_deps) $< &&\
	$(cc) -c -Winvalid-pch -o $@ $<
	
$(src): src/pch.h.gch

src/pch.h.gch: src/pch.h
	$(cc) $(include_flags) -MM -MF dep/pch.d -MQ $@ $^
	$(cc) $(include_flags) $^

vendor/%.o: vendor/%.c
	$(cc) -c -o $@ $^

.PHONY: clean
clean:
	- rm $(wildcard obj/*.o) $(deps) $(binary) src/pch.h.gch

include $(includes)

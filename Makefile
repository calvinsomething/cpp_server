objects = obj/main.o obj/server.o
deps = vendor/picohttpparser/picohttpparser.o

bin/server: $(objects) $(deps)
	g++ -pthread $(objects) $(deps) -o bin/server

obj/main.o: src/main.cpp
	g++ -c src/main.cpp -o obj/main.o -I vendor

obj/server.o: src/server.cpp
	g++ -c src/server.cpp -o obj/server.o -I vendor

vendor/picohttpparser/picohttpparser.o: vendor/picohttpparser/picohttpparser.h vendor/picohttpparser/picohttpparser.c
	g++ -c vendor/picohttpparser/picohttpparser.c -o vendor/picohttpparser/picohttpparser.o

.PHONY: clean
clean:
	- rm $(objects) bin/server

clean-deps:
	- rm $$(find vendor -name *.o)
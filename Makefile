objects = obj/main.o obj/Server.o obj/Mux.o obj/Worker.o
deps = vendor/picohttpparser/picohttpparser.o

bin/server: $(objects) $(deps)
	g++ -pthread $(objects) $(deps) -o bin/server

obj/main.o: src/main.cpp
	g++ -c src/main.cpp -o obj/main.o -I vendor

obj/Server.o: src/Server.cpp src/AtomicQueue.h
	g++ -c src/Server.cpp -o obj/Server.o -I vendor

obj/Mux.o: src/Mux.cpp src/Server.h src/Worker.h
	g++ -c src/Mux.cpp -o obj/Mux.o -I vendor

obj/Worker.o: src/Worker.cpp src/Server.h
	g++ -c src/Worker.cpp -o obj/Worker.o -I vendor

vendor/picohttpparser/picohttpparser.o: vendor/picohttpparser/picohttpparser.h vendor/picohttpparser/picohttpparser.c
	g++ -c vendor/picohttpparser/picohttpparser.c -o vendor/picohttpparser/picohttpparser.o

.PHONY: clean
clean:
	- rm $(objects) bin/server

clean-deps:
	- rm $$(find vendor -name *.o)
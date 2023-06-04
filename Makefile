objects = obj/server.o

bin/server: $(objects)
	g++ $(objects) -o bin/server

obj/server.o:
	g++ -c src/server.cpp -o obj/server.o

.PHONY: clean
clean:
	- rm $(objects) bin/server
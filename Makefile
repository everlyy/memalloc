run: build
	./bin/main
	
build: clean
	gcc -o bin/main src/main.c src/memalloc.c

clean:
	rm -f bin/main
	mkdir bin/
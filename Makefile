run: build
	./bin/main
	
build: clean
	gcc -o bin/main src/main.c src/memalloc.c

clean:
	rm -rf bin/
	mkdir bin/

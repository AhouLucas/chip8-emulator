main: src/main.c src/chip8.c src/stack.c
	gcc src/main.c src/chip8.c src/stack.c -o build/main -lraylib -Iheaders -Wall -std=c99 -g

clean:
	rm build/*
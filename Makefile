CC=gcc
CFLAGS=-Wall -ljson-c -lm

tool: src/main.c src/bg3d.c src/arg.c
	$(CC) $(CFLAGS) src/main.c -o tool

clean: tool
	rm -f tool

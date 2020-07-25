CC=gcc
CFLAGS=-Wall

viewer: src/main.c
	$(CC) $(CFLAGS) src/main.c -o reporter

clean: reporter
	rm -f reporter

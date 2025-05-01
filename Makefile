wimey_example: wimey.h wimey.c

wimey_example: wimey.c example.c
	$(CC) -Wall -W -Os -g -std=c99 -o wimey_example wimey.c example.c

clean:
	rm -f wimey_example

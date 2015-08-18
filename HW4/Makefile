all:httpserver

httpserver:httpserver.c
	clang -lpthread -o httpserver httpserver.c

clean:
	rm -rf *.o

clobber:clean
	rm -rf httpserver


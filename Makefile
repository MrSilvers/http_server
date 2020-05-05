http_server: http_server.o
	gcc -o http_server $^
	rm -f *.o
http_server.o: http_server.c request.h
	gcc -c $<

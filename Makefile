CC = g++ 
CFLAGS = -Wall -W
TARGETS = server-http
 
all: $(TARGETS)

server-http: server-http.o sockwrap.o utility.o

clean:
	rm -f *.o	

distclean: clean
	rm -f $(TARGETS)

#MakeFile to build and deploy the Sample US CENSUS Name Data using ajax
# For CSC3004 Software Development

# Put your user name below:
USER= bakboukg

CC= g++

#For Optimization
#CFLAGS= -O2
#For debugging
CFLAGS= -g

RM= /bin/rm -f

all: wordserver testclient wordfetchajax PutCGI PutHTML
#all: nameserver testclient 

testclient.o: testclient.cpp fifo.h
	$(CC) -c $(CFLAGS) testclient.cpp

wordserver.o: wordserver.cpp fifo.h
	$(CC) -c $(CFLAGS) wordserver.cpp

wordfetchajax.o: wordfetchajax.cpp fifo.h
	$(CC) -c $(CFLAGS) wordfetchajax.cpp

testclient: testclient.o fifo.o
	$(CC) testclient.o fifo.o -o testclient

wordserver: wordserver.o fifo.o
	$(CC) wordserver.o  fifo.o -o wordserver

fifo.o:		fifo.cpp fifo.h
		g++ -c fifo.cpp
wordfetchajax: wordfetchajax.o  fifo.h
	$(CC) wordfetchajax.o  fifo.o -o wordfetchajax -L/usr/local/lib -lcgicc

PutCGI: wordfetchajax
	chmod 757 wordfetchajax
	cp wordfetchajax /usr/lib/cgi-bin/$(USER)_wordfetchajax.cgi 

	echo "Current contents of your cgi-bin directory: "
	ls -l /usr/lib/cgi-bin/

PutHTML:
	cp word_stats_ajax.html /var/www/html/class/softdev/$(USER)
	cp word_stats_ajax.css /var/www/html/class/softdev/$(USER)

	echo "Current contents of your HTML directory: "
	ls -l /var/www/html/class/softdev/$(USER)

clean:
	rm -f *.o word_stats_ajax wordserver testclient

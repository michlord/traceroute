CC = gcc 
CFLAGS = -Wall -W -Wshadow -std=gnu99
TARGETS = traceroute
 
all: $(TARGETS)

traceroute: cksum.o probe.o probe_send.o probe_recv.o time.o traceroute.o arr_util.o print_output_for_poor.o

clean:
	rm -f *.o	

distclean: clean
	rm -f $(TARGETS)
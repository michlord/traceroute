Traceroute
===

This is a traceroute program implementation in C language.
I've made this program for a computer networks course at my university.
It uses ICMP packets to do the traceroute.
One subtlety of this implementation is that responses to
the 'probe' packets are stored in linked lists which allows more
than one response per packet event though this should never happen
in practice.
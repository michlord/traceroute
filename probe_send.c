#include "probe.h"

int set_ttl(int socket_fd, int ttl){
    if(setsockopt(socket_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof (int)) < 0){
        return ERROR_SET_SOCKET_TTL;
    }
    return ERROR_SUCCESS;
}

int send_probe_packet(int socket_fd, const void *data, size_t len, const struct sockaddr_in *addr){
    if(sendto(socket_fd, data, len, 0, (struct sockaddr*) addr, sizeof(*addr)) != (int)len){
        return ERROR_SEND_PROBE;
    }
    return ERROR_SUCCESS;
}
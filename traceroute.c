#include "probe.h"

#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <arpa/inet.h>

#include <sys/select.h>

#include <math.h>


double get_time();
void print_result_for_poor(struct t_probe *probes, int start, int count);

static void wait_and_recv(int socket_fd, double interval, int allowed_ttl){
    struct t_probe  *probe;
    
    fd_set           sockset;
    int              res_select;
    
    int              res_recv;
    int              reply_cnt = 0;
    
    int              last_time = get_time();
    struct timeval   tv;
    
    while(reply_cnt < PROBES_PER_TTL && last_time + interval > get_time()){
        FD_ZERO(&sockset);
        FD_SET(socket_fd, &sockset);
        
        tv.tv_sec = (int)(get_time() - last_time + interval);
        tv.tv_usec = (int)(1000000.0 * get_time() - 1000000.0 *\
                     last_time + 1000000.0 * interval) -\
                     1000000 * tv.tv_sec;
        
        res_select = select(socket_fd + 1, &sockset, 0, 0, &tv);

        if(res_select == -1){
            printf("Error while waiting for reply\n");
            exit(1);
        } else if(res_select == 0){
            /*printf("Timeout\n");*/
        } else{
            res_recv = recv_probe(socket_fd, &probe, allowed_ttl);
            /*printf("res_recv: %d\n",res_recv);*/
            if(res_recv == ERROR_SUCCESS){
                ++reply_cnt;
            }
        }
    }
}

static void traceroute(struct sockaddr_in *remote_address){
    int             ttl = 1;
    int             i;
    int             j;
    int             recv_socket_fd;
    int             res;
    
    res = init_icmp(remote_address);
    if(res != ERROR_SUCCESS){
        printf("Error initializing ICMP socket. Do you have admin rights?\n");
        exit(1);
    }
    
    recv_socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    
    for(j = 0; j < PROBES_COUNT; j += PROBES_PER_TTL){
        for( i = 0; i < PROBES_PER_TTL; ++i){
            send_probe(&probes_arr[j+i], ttl);
        }
        wait_and_recv(recv_socket_fd, 1.0, ttl);
        
        /*print_probes(probes_arr, j, PROBES_PER_TTL);*/
        print_result_for_poor(probes_arr, j, PROBES_PER_TTL);
        
        if(is_target_reached()){
            return;
        }
        
        ++ttl;
    }
}


int cstring_to_sockaddr_in(const char *ip_address, struct sockaddr_in *sa){
    bzero(sa, sizeof(*sa));
    sa->sin_family = AF_INET;
    
    return  inet_pton(AF_INET, ip_address, &(sa->sin_addr));
}

int main(int argc, char** argv){
    int res;

    if(argc != 2){
        printf("Please provide only the ip address to traceroute to.\n");
        return 1;
    }

    struct sockaddr_in remote_addr;
    res = cstring_to_sockaddr_in(argv[1], &remote_addr);
    if(res == 0){
        printf("Provided ip address is invalid in specified address family.\n");
        return 1;
    } else if(res != 1){
        printf("Provided ip address is invalid.\n");
        return 1;
    }
    
    traceroute(&remote_addr);

    return 0;
}

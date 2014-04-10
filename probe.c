#include "probe.h"

#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#include <stdlib.h>
#include <string.h>

int send_probe_packet(int socket_fd, const void *data, size_t len, const struct sockaddr_in *addr);
int set_ttl(int socket_fd, int ttl);

int recv_reply(int socket_fd, struct t_reply *reply, int *seq);

u_short in_cksum(const u_short *addr, register int len, u_short csum);
double get_time();

static struct sockaddr_in dest_addr;
static int    cur_seq = 0;
static size_t packet_length = 8;
static int    icmp_sk_fd;
static int    last_ttl = 0;

pid_t  prog_pid;

int init_icmp(const struct sockaddr_in *dest){
    dest_addr = *dest;
    dest_addr.sin_port = 0;
    
    icmp_sk_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    
    if(icmp_sk_fd < 0){
        return ERROR_OPEN_SOCKET;
    }
    
    prog_pid = getpid();
    
    return ERROR_SUCCESS;
}

int send_probe(struct t_probe *probe, int ttl){
    struct icmp icmp_packet;
    
    if(ttl != last_ttl){
        set_ttl(icmp_sk_fd, ttl);
        last_ttl = ttl;
    }
    
    icmp_packet.icmp_type  = ICMP_ECHO;
    icmp_packet.icmp_code  = 0;
    icmp_packet.icmp_id    = htons(prog_pid);
    icmp_packet.icmp_seq   = htons(cur_seq);
    icmp_packet.icmp_cksum = 0;
    icmp_packet.icmp_cksum = in_cksum((u_short*)&icmp_packet, packet_length, 0);
    
    probe->seq         = cur_seq;
    probe->send_time   = get_time();
    probe->ttl         = last_ttl;
    probe->replies_cnt = 0;
    probe->first_reply = 0;
    
    if (send_probe_packet(icmp_sk_fd, &icmp_packet, packet_length, &dest_addr) != ERROR_SUCCESS){
        probe->send_time = 0;
        return ERROR_SEND_PROBE;
    }
    
    ++cur_seq;
    
    return ERROR_SUCCESS;
}

static void add_reply(struct t_probe *probe, struct t_reply *reply){
    struct t_reply *next;

    if(probe->first_reply == 0){
        probe->first_reply = malloc(sizeof(struct t_reply));
        *(probe->first_reply) = *reply;
        probe->first_reply->next_reply = 0;
        probe->replies_cnt = 1;
    } else {
        next = probe->first_reply;
        while(next->next_reply != 0){
            next = next->next_reply;
        }
        next->next_reply = malloc(sizeof(struct t_reply));
        *(next->next_reply) = *reply;
        next->next_reply->next_reply = 0;
        probe->replies_cnt += 1;
    }
}

static int target_reached_flag = 0;

static void update_target_reached_flag(struct t_reply *reply){
    if(reply->type == ICMP_ECHOREPLY){
        if(reply->recv_addr.sin_family == AF_INET){
            target_reached_flag = target_reached_flag || (reply->recv_addr.sin_addr.s_addr == dest_addr.sin_addr.s_addr);
        }
    }
}

int is_target_reached(){
    return target_reached_flag;    
}

int recv_probe(int socket_fd, struct t_probe **probe, int allowed_ttl){
    struct t_reply  reply;
    int             seq;
    int             res;
    
    if((res = recv_reply(socket_fd, &reply, &seq)) != ERROR_SUCCESS){
        return res;
    }
    
    *probe = get_probe_by_seq(seq);
    if(*probe == 0){
        return ERROR_RECV_PROBE_SEQ_NOT_FOUND;
    }
    
    /*
    * 'allowed_ttl == 0' means that every ttl value is allowed.
    */
    if(!allowed_ttl){
        if((*probe)->ttl == allowed_ttl){
            return ERROR_RECV_REPLY_DISCARDED_TTL;
        }
    }
    
    add_reply(*probe, &reply);
    
    update_target_reached_flag(&reply);
    
    return ERROR_SUCCESS;
}

struct t_probe probes_arr[PROBES_COUNT];

struct t_probe *get_probe_by_seq(int seq){
    int i;
    for(i = 0; i < PROBES_COUNT; ++i){
        if(probes_arr[i].seq == seq){
            return &(probes_arr[i]);
        }
    }
    return 0;
}


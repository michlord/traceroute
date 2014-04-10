#include "probe.h"

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

double get_time();

static int             icmp_header_len = 8;
static int             ip_maxpacket = IP_MAXPACKET;
static unsigned char   buffer[IP_MAXPACKET + 1];

static void consume_chunk(unsigned char **buff, int *rem_count, int chunk_len){
    *buff     += chunk_len;
    rem_count -= chunk_len;
}

static void consume_ip(unsigned char **buff, int *rem_count){
    struct ip *packet = (struct ip*)(*buff);
    consume_chunk(buff, rem_count, packet->ip_hl * 4);
}

static void consume_icmp1(unsigned char **buff, int *rem_count, unsigned short int *type, unsigned short int *code,
                          unsigned short int *op_id, unsigned short int *op_seq){
    struct icmp *icmp_packet = (struct icmp*)(*buff);
    *type = icmp_packet->icmp_type;
    *code = icmp_packet->icmp_code;
    
    /* This is undefined if it wasn't echo reply. */
    *op_id  = ntohs(icmp_packet->icmp_id);
    *op_seq = ntohs(icmp_packet->icmp_seq);

    consume_chunk(buff, rem_count, icmp_header_len);
}

static void consume_icmp2(unsigned char **buff, int *rem_count, unsigned short int *id, unsigned short int *seq){
    struct icmp *icmp_packet;
    struct ip   *packet_orig = (struct ip*)(*buff);
    
    consume_chunk(buff, rem_count, packet_orig->ip_hl * 4);
    
    icmp_packet = (struct icmp*)(*buff);
    *id  = ntohs(icmp_packet->icmp_id);
    *seq = ntohs(icmp_packet->icmp_seq);
    
    consume_chunk(buff, rem_count, icmp_header_len);    
}

int recv_reply(int socket_fd, struct t_reply *reply, int *seq){   
    int                recv_count = 0;
    int                rem_count;
    struct sockaddr_in addr;
    size_t             addr_len = sizeof(addr);
    unsigned char     *buffer_p = buffer;
    
    unsigned short     recv_type;
    unsigned short     recv_code;
    unsigned short     recv_seq;
    unsigned short     recv_id;
    
    if((recv_count = recvfrom(socket_fd, buffer_p, ip_maxpacket, 0, (struct sockaddr*)&addr, (socklen_t *)&addr_len)) < 0){
        return ERROR_RECV_REPLY_READ;
    }
    rem_count = recv_count;
    
    consume_ip(&buffer_p,&rem_count);
    consume_icmp1(&buffer_p,&rem_count,&recv_type,&recv_code,&recv_id,&recv_seq);

    if( (recv_type == ICMP_TIME_EXCEEDED && recv_code != ICMP_EXC_TTL) ||
        (recv_type != ICMP_TIME_EXCEEDED && recv_type != ICMP_ECHOREPLY)){
        return ERROR_RECV_REPLY_WRONG_TYPE;
    }
    
    if(recv_type != ICMP_ECHOREPLY){
        consume_icmp2(&buffer_p,&rem_count,&recv_id,&recv_seq);
    }
    
    if(recv_id != prog_pid){
        return ERROR_RECV_REPLY_WRONG_ID;
    }
    
    reply->recv_addr  = addr;
    reply->recv_time  = get_time();
    reply->type       = recv_type;
    reply->next_reply = 0;
    *seq              = recv_seq;
    
    /*printf("type: %d, seq: %d\n",recv_type,recv_seq);*/
    
    return ERROR_SUCCESS;    
}

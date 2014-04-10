#include "probe.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <math.h>

static void print_probe(struct t_probe *probe){
    char                 ip_present_str[256];
    struct t_reply      *next_reply;
    
    if(probe->replies_cnt == 0){
        printf("* ");
    } else {
        next_reply = probe->first_reply;
        while(next_reply != 0){
            inet_ntop(AF_INET, &(next_reply->recv_addr.sin_addr), ip_present_str, sizeof(ip_present_str)*sizeof(char));
            printf("%s %.2lfms ", ip_present_str, (next_reply->recv_time*1000.0 - probe->send_time*1000.0));
            next_reply = next_reply->next_reply;
        }
    }
}

static double get_probes_average_time(struct t_probe *probes, int start, int count){
    double          average_time = 0;
    int             replies_cnt_total = 0;
    int             i;
    struct t_probe *probe;
    struct t_reply *next_reply;
    
    for(i = start; i < count+start; ++i){
        probe = probes+i;
        replies_cnt_total += probe->replies_cnt;
        if(probe->replies_cnt == 0){
            continue;
        } else {
            next_reply = probe->first_reply;
            while(next_reply != 0){
                average_time += next_reply->recv_time*1000.0 - probe->send_time*1000.0;
                next_reply = next_reply->next_reply;
            }
        }
    }
    return average_time/(double)replies_cnt_total;
}

void print_probes(struct t_probe *probes, int start, int count){
    int    i;
    int    ttl = probes[start].ttl;
    double average_time;
    
    printf("%d. ",ttl);
    for(i = start; i < count+start; ++i){
        print_probe(probes+i);
    }
    average_time = get_probes_average_time(probes,start,count);
    if(!isnan(average_time)){
        printf("avg: %.2lfms\n", average_time);
    } else {
        printf("avg: ???\n");
    }
}
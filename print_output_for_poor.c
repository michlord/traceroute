#include "probe.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void unique_addr_user_free_mem(struct t_probe *probes, int start, int count, unsigned long **out_arr, int *out_arr_size);

static double get_probes_average_time_for_poor(struct t_probe *probes, int start, int count){
    double          average_time = 0;
    int             replies_cnt_total = 0;
    int             i;
    struct t_probe *probe;
    struct t_reply *next_reply;
    
    for(i = start; i < count+start; ++i){
        probe = probes+i;
        replies_cnt_total += probe->replies_cnt;
        if(probe->replies_cnt == 0){
            average_time = -1.0;
            return average_time;
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

void print_result_for_poor(struct t_probe *probes, int start, int count){
    double              average_time = get_probes_average_time_for_poor(probes, start, count);
    unsigned long*      addr_long_arr;
    int                 addr_long_arr_size;
    int                 i;
    char                ip_present_str[256];
    struct in_addr      sin_addr;
    
    int                 ttl = probes[start].ttl;
    
    unique_addr_user_free_mem(probes, start, count, &addr_long_arr, &addr_long_arr_size);
    
    printf("%d. ", ttl);
    
    if(addr_long_arr_size == 0){
        printf("* ");
    }
    
    for(i = 0; i < addr_long_arr_size; ++i){
        sin_addr.s_addr = addr_long_arr[i];
        inet_ntop(AF_INET, &sin_addr, ip_present_str, sizeof(ip_present_str)*sizeof(char));
        printf("%s ",ip_present_str);
        
    }
    if(average_time == -1.0 || isnan(average_time)){
        printf("??? ");
    } else {
        printf("%dms ", (int)average_time);
    }
    
    printf("\n");
    
    free(addr_long_arr); 
}

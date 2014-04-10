#include <string.h>
#include <stdlib.h>
#include "probe.h"


/*http://en.cppreference.com/w/cpp/algorithm/unique*/
static unsigned long *unique(unsigned long *first, unsigned long *last){
    if(first == last) return last;
    unsigned long* result = first;
    while(++first != last){
        if(!(*result == *first)) 
            *(++result)=*first;
    }
    return ++result;
}

static int compare_unsigned_long(const void *a, const void *b)
{
    if(*(unsigned long*)a < *(unsigned long*)b) return -1;
    if(*(unsigned long*)a == *(unsigned long*)b) return 0;
    return 1;
}

void unique_addr_user_free_mem(struct t_probe *probes, int start, int count, unsigned long **out_arr, int *out_arr_size){
    int             i;
    struct t_probe *probe;
    struct t_reply *next_reply;
    int             replies_cnt_total = 0;
    int             j = 0;
    unsigned long  *tmp_arr;
    
    for(i = start; i < count+start; ++i){
        probe = probes+i;
        replies_cnt_total += probe->replies_cnt;
        if(probe->replies_cnt == 0){
            continue;
        } else {
            next_reply = probe->first_reply;
            while(next_reply != 0){
                next_reply = next_reply->next_reply;
            }
        }
    }
    
    tmp_arr = malloc(sizeof(unsigned long)*replies_cnt_total);
    
    for(i = start; i < count+start; ++i){
        probe = probes+i;
        if(probe->replies_cnt == 0){
            continue;
        } else {
            next_reply = probe->first_reply;
            while(next_reply != 0){
                tmp_arr[j] = next_reply->recv_addr.sin_addr.s_addr;
                ++j;
                next_reply = next_reply->next_reply;
            }
        }
    }
    
    qsort(tmp_arr, replies_cnt_total, sizeof(unsigned long), compare_unsigned_long);
    *out_arr_size = (int)(unique(tmp_arr,tmp_arr+replies_cnt_total) - tmp_arr);
    *out_arr = malloc(sizeof(unsigned long)*(*out_arr_size));
    memcpy(*out_arr,tmp_arr,sizeof(unsigned long)*(*out_arr_size));
    free(tmp_arr);    
}
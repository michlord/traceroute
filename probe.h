#ifndef PROBE_H_
#define PROBE_H_

#include <unistd.h>
#include <netinet/in.h>

struct t_reply{
    struct sockaddr_in  recv_addr;
    double              recv_time;
    int                 type;
    struct t_reply      *next_reply;
};

struct t_probe{
    double              send_time;
    int                 seq;
    int                 replies_cnt;
    int                 ttl;
    struct t_reply      *first_reply;
};

int              is_target_reached();
int              init_icmp(const struct sockaddr_in *dest);
int              send_probe(struct t_probe *probe, int ttl);
int              recv_probe(int socket_fd, struct t_probe **probe, int allowed_ttl);
struct t_probe  *get_probe_by_seq(int seq);

#define MAX_HOPS 30
#define PROBES_PER_TTL 3
#define PROBES_COUNT (MAX_HOPS*PROBES_PER_TTL)

extern pid_t           prog_pid;
extern struct t_probe  probes_arr[PROBES_COUNT];

#define ERROR_SUCCESS 0

#define ERROR_OPEN_SOCKET 1

#define ERROR_SET_SOCKET_TTL 1

#define ERROR_SEND_PROBE 1

#define ERROR_RECV_REPLY_READ 1
#define ERROR_RECV_REPLY_WRONG_TYPE 2
#define ERROR_RECV_REPLY_WRONG_ID 4
#define ERROR_RECV_REPLY_DISCARDED_TTL 8

#define ERROR_RECV_PROBE_SEQ_NOT_FOUND 16
#define ERROR_RECV_PROBE_READ_ERROR 32

#endif //PROBE_H_
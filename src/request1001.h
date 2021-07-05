#include <sys/socket.h>

#define ESIZE 550
#define LSIZE 48

// Levin 1001 data struct
struct data {
        unsigned char d[226];
} __attribute__((packed));

// Methods
void * start1001(void *args);
void * hautatu_enodoa(struct bst_ip *un, FILE *log, time_t denb);
void * eskatu_ip(char * target, int port, FILE *log);

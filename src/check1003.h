#include <sys/socket.h>

// Levin 1003 data struct
struct data1003 {
	unsigned char d[10];
} __attribute__((packed));


// Methods
void * start1003(void *args);
void * hautatu_knodoa(struct bst_ip *un, FILE *log, time_t denb);
void * konprobatu_ping(char *target, int port, FILE *log);


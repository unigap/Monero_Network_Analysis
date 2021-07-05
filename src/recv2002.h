#include <sys/socket.h>

struct datatr {
	unsigned char d[226];
} __attribute__((packed));

// Methods
void * recv_transacts(void *args);
void * hasieratu_trnodoa(void *args);
void * hautatu_tnodoa(struct bst_ip *un, int id, FILE *log);
void * jaso2002(struct bst_ip *un, FILE *log, int id);

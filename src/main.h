#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <linux/kernel.h>
#include <poll.h>
#include <fcntl.h>
#include <stdbool.h>
#include <signal.h>

#define B_RED "\033[1m\033[31m"
#define RESET "\033[0m"


// Levin header struct declaration
struct levhdr {
        __be64          sign;           // Signature of msg (8 bytes)   /  0x0121010101010101 (beti)
        __le64          length;         // Length of msg (8 bytes)      /  0xe2=226
        unsigned char   exp_resp;       // Expected response (1 byte)   /  0x01 (request)
        __le32          comm_cod;       // Command code (4 bytes)       /  0xe903=1001 (handshake), 0xeb03=1003 (ping), 0xef03=1007 (support flags)...
        __le32          retn_cod;       // Return code (4 bytes)        /  0x00000000
        __le32          reserved;       // Reserved (4 bytes)           /  0x01000000
        __be32          endchars;       // Ending chars (4 bytes)       /  0x01000000 (beti)
} __attribute__((packed)); // 33 bytes packed 

// Methods + int main()
void init(char *argv1, char *argv2);
int connect_with_timeout(int sockfd, const struct sockaddr *addr, socklen_t addrlen, unsigned int timeout_ms);

